use image::{ImageBuffer, Rgba};
use anyhow::Result;
use rand::{rng, seq::{index, SliceRandom}};
use rayon::prelude::*;
use std::f64::consts::PI;
use itertools::Itertools;
use imageproc::drawing::draw_filled_circle_mut;
use std::fs::File;
use std::io::Write;

const SIZE: i32 = 64;
const NCOLLS: i32 = 2000;
const DIST_FACTOR: f64 = 20.0;
const PITCH: f64 = 2.5;
const PANEL_2_OFFSET: f64 = 15.0/PITCH;
const IMAGE_SCALE: i32 = 12;

#[derive(Debug, Clone, Copy, PartialEq)]
struct Point {
    x: f64,
    y: f64,
    z: f64,
}

impl Point{
    fn new(x: f64, y: f64, z: f64) -> Self {
        Point { x, y, z}
    }
    fn distance(&self, other: &Point) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        let dz = self.z - other.z;
        (dx * dx + dy * dy + dz * dz).sqrt()
    }
    fn from_angle(index: i32, side: bool, angle: f64, offset: f64) -> Self {
        let panel_center_dist = (index as f64 - (if side {(SIZE as f64) / 2.0} else {0.0})) + 0.5;
        let z = if offset == 0.0 {0.0} else {0.5};
        if offset == 0.0 {
            let x = panel_center_dist * angle.cos();
            let y = panel_center_dist * angle.sin();
            return Point::new(x, y, z)
        } else{
            let x = -offset * angle.sin() - panel_center_dist * angle.cos();
            let y = offset * angle.cos() - panel_center_dist * angle.sin();
            return Point::new(x, y, z)
        }
    }
    fn nearest_point(&self, points: &Vec<Point>) -> f64 {
        let mut nearest_distance = f64::MAX;
        for point in points.iter() {
            let distance = self.distance(point);
            if distance < nearest_distance {
                nearest_distance = distance;
            }
        }
        nearest_distance
    }
    fn dist_to_origin(&self) -> f64 {
        (self.x * self.x + self.y * self.y).sqrt()
    }
    fn normal(side: bool, angle: f64) -> Self {
        if side {
            return Point::new(angle.sin(), -angle.cos(), 0.0);
        } else {
            return Point::new(-angle.sin(), angle.cos(), 0.0);
        }
    }
}

fn save_img(img: &ImageBuffer<Rgba<u8>, Vec<u8>>, path: &str) -> Result<()> {
    img.save(path)?;
    Ok(())
}

fn product<T: Clone>(items: &[T], repeat: usize) -> Vec<Vec<T>> {
    if repeat == 0 {
        return vec![vec![]];
    }

    let iterators = vec![items.iter(); repeat];

    iterators
        .into_iter()
        .multi_cartesian_product()
        .map(|vec_of_refs| vec_of_refs.into_iter().cloned().collect::<Vec<T>>())
        .collect()
}

fn write_to_file(path: &str, best: &Vec<(i32, Best)>) -> Result<()> {
    let mut file = File::create(path)?;
    for (frame, best_item) in best.iter() {
        let index1 = best_item.index1;
        let index2 = best_item.index2;
        let pattern = best_item.pattern;
        writeln!(file, "{} {} {} {} {} {} {}", frame, index1, index2, pattern.0 as i32, pattern.1 as i32, pattern.2 as i32, pattern.3 as i32)?;
    }
    Ok(())
}

fn write_pointcloud_to_file(path: &str, points: &Vec<Point>, normals: &Vec<Point>) -> Result<()> {
    let mut file = File::create(path)?;
    writeln!(file, "ply\nformat ascii 1.0")?;
    writeln!(file, "element vertex {}", points.len()*64)?;
    writeln!(file, "property float x\nproperty float y\nproperty float z")?;
    writeln!(file, "property float nx\nproperty float ny\nproperty float nz")?;
    writeln!(file, "end_header")?;
    for i in 0..points.len() {
        let pt = points[i];
        let normal = normals[i];
        for j in 0..64 {
            write!(file, "{} {} {} ", pt.x, pt.y, pt.z+j as f64)?;
            writeln!(file, "{} {} {}", normal.x, normal.y, normal.z)?;
        }
    }
    Ok(())
}

#[derive(Debug, Clone)]
struct Best {
    index1: i32,
    index2: i32,
    pattern: (bool, bool, bool, bool),
    pts1: Vec<Point>,
    pts2: Vec<Point>,
    normal1: Point,
    normal2: Point,
}

fn main() {
    let start = std::time::Instant::now();
    let mut rng = rng();

    let mut best_choices: Vec<(i32, Best)> = vec![];

    let mut pts: Vec<Point> = vec![];

    let mut normals: Vec<Point> = vec![];

    let mut img: ImageBuffer<Rgba<u8>, Vec<u8>> = ImageBuffer::new((SIZE*IMAGE_SCALE) as u32, (SIZE*IMAGE_SCALE) as u32);

    let mut n = 0;

    let mut r = (0..NCOLLS).collect::<Vec<i32>>();
    r.shuffle(&mut rng);

    for frame in r {
        let angle = (frame as f64) / (NCOLLS as f64) * PI * 2.0;
        
        let best_choice = (0..(1<<10 as i32)).into_par_iter().map(|index_encoded| {
            let index1 = index_encoded >> 5;
            let index2 = index_encoded & 0b11111;
            let mut best_choice_local: Best = Best { index1: 0, index2:0, pattern: (false, false, false, false), pts1: vec![], pts2: vec![] , normal1: Point::new(0.0, 0.0, 0.0), normal2: Point::new(0.0, 0.0, 0.0)};
            let mut max_score_local: f64 = -1.0;
            for activation_pattern in product(&[true, false], 4).iter() {
                if activation_pattern.iter().all(|&x| x == false) {
                    continue;
                }
                let mut pts1: Vec<Point> = vec![];
                let mut pts2: Vec<Point> = vec![];

                for (i, active) in activation_pattern.iter().enumerate() {
                    if !*active {
                        //pts1.push(Point::new(0.0, 0.0, 0.0))
                        continue;
                    }
                    else if i<2 {
                        pts1.push(Point::from_angle(index1, !(i%2==0), angle, 0.0));
                    }
                    else {
                        pts2.push(Point::from_angle(index2, !(i%2==0), angle, PANEL_2_OFFSET));
                    }
                }
                let mut score: f64 = 0.0;
                for ptgroup in [&pts1, &pts2].iter() {
                    let dist_f = if ptgroup == &&pts1 {10.0} else {0.0};
                    if ptgroup.iter().all(|p| p.x == 0.0 && p.y == 0.0) {
                        continue;
                    }
                    if pts.len() == 0 {
                        score += 1000.0;
                        continue;
                    }
                    if ptgroup.len() == 1 {
                        let pt = ptgroup[0];
                        score += pt.nearest_point(&pts).powf(2.0) * (dist_f + pt.dist_to_origin().powf(1.2));
                        //println!("base {}, origin {}", pt.nearest_point(&pts).powf(2.0), pt.dist_to_origin()*10.0);
                    }
                    else {
                        score += ptgroup.iter().map(|p| p.nearest_point(&pts).powf(2.0) * (dist_f + p.dist_to_origin().powf(1.2))).sum::<f64>() * 0.9;
                    }

                }

                if score > max_score_local {
                    max_score_local = score;
                    best_choice_local = Best { 
                        index1, 
                        index2, 
                        pattern: (activation_pattern[0], activation_pattern[1], activation_pattern[2], activation_pattern[3]), 
                        pts1: pts1.clone(), 
                        pts2: pts2.clone(),
                        normal1: Point::normal(true, angle),
                        normal2: Point::normal(false, angle),
                    };
                }
            }
            (best_choice_local, max_score_local)
        }).max_by(|a, b| a.1.partial_cmp(&b.1).unwrap()).unwrap().0;

        best_choices.push((frame, best_choice.clone()));
        let Best { index1: _index1, index2: _index2, pattern, pts1: new_pts1, pts2: new_pts2, normal1: new_normal1, normal2: new_normal2 } = best_choice;

        n += pattern.0 as i32 + pattern.1 as i32 + pattern.2 as i32 + pattern.3 as i32;

        for pt in new_pts1.iter().chain(new_pts2.iter()) {
            pts.push(*pt);
            normals.push(if new_pts1.contains(pt) {new_normal1} else {new_normal2});
            let img_x = pt.x * (IMAGE_SCALE as f64) + ((SIZE * IMAGE_SCALE) as f64) / 2.0;
            let img_y = pt.y * (IMAGE_SCALE as f64) + ((SIZE * IMAGE_SCALE) as f64) / 2.0;
            let color = if new_pts1.contains(pt) {Rgba([255, 0, 0, 255])} else {Rgba([0, 255, 0, 255])};
            draw_filled_circle_mut(&mut img, (img_x as i32, img_y as i32), 2, color);
        }
    }
    // Histogram analysis
    let num_bins = 20; // Adjustable
    let max_radius = 32.0; // Half of max diameter (~34)
    let bin_width = max_radius / num_bins as f64;

    let mut bin_counts = vec![0; num_bins];
    let mut bin_areas = vec![0.0; num_bins];

    // Calculate bin areas (annulus areas)
    for i in 0..num_bins {
        let r_inner = i as f64 * bin_width;
        let r_outer = (i + 1) as f64 * bin_width;
        bin_areas[i] = std::f64::consts::PI * (r_outer * r_outer - r_inner * r_inner);
    }

    // Count points in each bin
    for pt in &pts {
        let r = pt.dist_to_origin();
        let bin_idx = (r / bin_width).floor() as usize;
        if bin_idx < num_bins {
            bin_counts[bin_idx] += 1;
        }
    }

    // Calculate and print density vs expected
    let total_points = pts.len() as f64;
    let total_area: f64 = bin_areas.iter().sum();

    println!("\nBin | Actual Density | Expected Density | Ratio");
    println!("----+----------------+------------------+-------");
    for i in 0..num_bins {
        let actual_density = bin_counts[i] as f64 / bin_areas[i];
        let expected_density = total_points / total_area;
        let ratio = actual_density / expected_density;
        println!("{:3} | {:14.4} | {:16.4} | {:5.2}", 
                i, actual_density, expected_density, ratio);
    }

    save_img(&img, "output.png").expect("Unable to save image");
    write_to_file("output.txt", &best_choices).expect("Unable to write to file");
    write_pointcloud_to_file("pointcloud.ply", &pts, &normals).expect("Unable to write point cloud to file");
    println!("{n}");
    let duration = start.elapsed();
    println!("Time elapsed: {:?}", duration);
}
