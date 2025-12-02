use image::{ImageBuffer, Rgba};
use anyhow::Result;
use rand::{rng, seq::SliceRandom};
use rand;
use rayon::prelude::*;
use std::f64::consts::PI;
use itertools::Itertools;
use imageproc::drawing::draw_filled_circle_mut;
use std::fs::File;
use std::io::Write;

const SIZE: i32 = 64;
const NCOLLS: i32 = 200;
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
    fn distance2(&self, other: &Point) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        let dz = self.z - other.z;
        dx * dx + dy * dy + dz * dz
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
    fn nearest_point(&self, points: &Vec<Point>, normal: &Point, normals: &Vec<Point>) -> f64 {
        let mut nearest_distance = f64::MAX;
        for i in 0..points.len() {
            if normal.dot(&normals[i]) < 0.0 {
                continue;
            }
            let distance = self.distance(&points[i]);
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
    fn dot(&self, other: &Point) -> f64 {
        self.x * other.x + self.y * other.y + self.z * other.z
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
fn energy(points: &Vec<Point>, indices: &Vec<usize>, index: usize, sigma: f64) -> f64 {
    // println!("finding energy for: {}, indices len: {}", index, indices.len());
    let divisor = 2.0*sigma*sigma;
    let pt: &Point = &points[index];
    let mut sum: f64 = 0.;
    for &i in indices {
        if i == index {
            continue;
        }
        let pt2 = &points[i];
        if
            (pt.x - pt2.x).abs() > 3.5*sigma ||
            (pt.y - pt2.y).abs() > 3.5*sigma ||
            (pt.z - pt2.z).abs() > 3.5*sigma
        { continue; }
        let d2 = pt.distance2(pt2);
        sum += (-d2 / divisor).exp();
    }
    sum
}
fn find_max_energy(points: &Vec<Point>, indices: &Vec<usize>, e_map: &mut Vec<f64>) -> usize {
    // println!("finding max energy");
    let mut max: f64 = 0.;
    let mut best: usize = indices[0];
    for &i in indices {
        let e_val = e_map[i];
        if e_val != f64::MIN {
            // if e_val <= 0. {
            //     e_val = energy(points, &indices, i, 1.9);
            //     e_map[i] = e_val;
            // }
            if e_val > max {
                max = e_val;
                best = i;
            }
        } else {
            let e = energy(points, &indices, i, 1.5);
            e_map[i] = e;
            if e > max {
                max = e;
                best = i;
            }
        }
    }
    best
}
fn remove_from_e_map(points: &Vec<Point>, e_map: &mut Vec<f64>, indices: &Vec<usize>, index: usize, sigma: f64) {
    e_map[index] = f64::MIN; // remove point from map
    let divisor = sigma*sigma*2.;
    for &i in indices {
        let pt = points[index];
        if e_map[i] != f64::MIN {
            let pt2 = &points[i];
            if
                (pt.x - pt2.x).abs() > 3.5*sigma ||
                (pt.y - pt2.y).abs() > 3.5*sigma ||
                (pt.z - pt2.z).abs() > 3.5*sigma
            { continue; }
            let d2 = pt.distance2(pt2);
            e_map[i] -= (-d2 / divisor).exp();
        }
    }
}
fn add_to_e_map(points: &Vec<Point>, e_map: &mut Vec<f64>, indices: &Vec<usize>, index: usize, sigma: f64) {
    let divisor = sigma*sigma*2.;
    for &i in indices {
        let pt = points[index];
        if e_map[i] != f64::MIN {
            let pt2 = &points[i];
            if
                (pt.x - pt2.x).abs() > 3.5*sigma ||
                (pt.y - pt2.y).abs() > 3.5*sigma ||
                (pt.z - pt2.z).abs() > 3.5*sigma
            { continue; }
            let d2 = pt.distance2(pt2);
            e_map[i] += (-d2 / divisor).exp();
        }
    }
    e_map[index] = energy(points, indices, index, sigma);
}
fn generate_dither(points: &Vec<Point>) -> Vec<f32> {
    println!("Generating dither blue noise");
    println!("Generating initial binary pattern");
    let mut rng = rng();
    //INITAL POPULATION
    let npoints = points.len();

    let mut e_map_selected: Vec<f64> = vec![f64::MIN; npoints];
    let mut e_map_unused:  Vec<f64> = vec![f64::MIN; npoints];

    let mut unused: Vec<usize> = (0..npoints).collect();
    let mut selected: Vec<usize> = vec![];
    unused.shuffle(&mut rng);
    let sample_size = npoints / 10;
    println!("unused len: {}, sample size: {}", unused.len(), sample_size);
    for &i in &unused.clone()[..sample_size] {
        selected.push(i);
        unused.retain(|&x| x != i);
    }
    println!("making initial binary pattern blue");
    let mut voidiest: usize = usize::MAX;
    let mut clusteriest: usize;
    loop {
        //get max cluster
        clusteriest = find_max_energy(points, &selected, &mut e_map_selected);
        if clusteriest == voidiest { break;}
        remove_from_e_map(points, &mut e_map_selected, &selected, clusteriest, 1.5);
        add_to_e_map(points, &mut e_map_unused, &unused, clusteriest, 1.5);


        selected.retain(|&x| x != clusteriest);
        unused.push(clusteriest);

        voidiest = find_max_energy(points, &unused, &mut e_map_unused);
        remove_from_e_map(points, &mut e_map_unused, &unused, voidiest, 1.5);
        add_to_e_map(points, &mut e_map_selected, &selected, voidiest, 1.5);

        unused.retain(|&x| x != voidiest);

        selected.push(voidiest);
        if clusteriest == voidiest { break; }
    }

    println!("Make Initial binary pattern progressive");
    let mut ranked: Vec<f32> = vec![0.; npoints];

    let mut selected_copy = selected.clone();
    let mut e_map_selected_copy = e_map_selected.clone();
    while selected_copy.len() > 0 {
        let clusteriest = find_max_energy(points, &selected_copy, &mut e_map_selected_copy);
        ranked[clusteriest] = selected_copy.len() as f32 / npoints as f32;
        remove_from_e_map(points, &mut e_map_selected_copy, &selected_copy, clusteriest, 1.5);
        selected_copy.retain(|&x| x != clusteriest);
    }

    println!("Fill remaining points");
    while selected.len() < npoints {
        if unused.len()%10000==0 {
            println!("remaining: {}", unused.len());
        }
        let voidiest = find_max_energy(points, &unused, &mut e_map_selected);
        ranked[voidiest] = selected.len() as f32 / npoints as f32;
        remove_from_e_map(points, &mut e_map_unused, &unused, voidiest, 1.5);
        unused.retain(|&x| x != voidiest);
        selected.push(voidiest);
    }
    // return a vector of thresholds (placeholder zeros) with length `n`
    ranked
}
fn write_to_file(path: &str, best: &Vec<(i32, Best)>, dither_ranks: &Vec<f32>) -> Result<()> {
    println!("num of dither ranks: {}", dither_ranks.len());

    let mut pt_index = 0;

    let mut file = File::create(path)?;
    for (frame, best_item) in best.iter() {
        let index1 = best_item.index1;
        let index2 = best_item.index2;
        let pattern = best_item.pattern;
        let mut pt_coords: Vec<Point> = best_item.pts1.clone();
        pt_coords.extend(&best_item.pts2);
        let next_str = format!("{} {} {} {} {} {} {}", frame, index1, index2, pattern[0] as i32, pattern[1] as i32, pattern[2] as i32, pattern[3] as i32);
        for slice_z in 0..64 {
            let mut i = 0;
            let mut ahoj = next_str.clone();
            for j in 0..4 {
                if pattern[j] == true {
                    ahoj.push_str(&format!(" {}|{},{},{}", dither_ranks[pt_index], pt_coords[i].x, pt_coords[i].y, pt_coords[i].z + slice_z as f64));
                    i+=1;
                    pt_index += 1;
                }
            }
            writeln!(file, "{}", ahoj)?;
        }
    }
    Ok(())
}

fn write_pointcloud_to_file(path: &str, points: &Vec<Point>, normals: &Vec<Point>) -> Result<()> {
    let mut file = File::create(path)?;
    let height: i32 = 64;
    writeln!(file, "ply\nformat ascii 1.0")?;
    writeln!(file, "element vertex {}", points.len() as i32 * height)?;
    writeln!(file, "property float x\nproperty float y\nproperty float z")?;
    writeln!(file, "property float nx\nproperty float ny\nproperty float nz")?;
    writeln!(file, "end_header")?;
    for i in 0..points.len() {
        let pt = points[i];
        let normal = normals[i];
        for j in 0..height {
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
    pattern: [bool; 4],
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

    let mut all_pts: Vec<Point> = vec![];

    let mut normals: Vec<Point> = vec![];

    let mut img: ImageBuffer<Rgba<u8>, Vec<u8>> = ImageBuffer::new((SIZE*IMAGE_SCALE) as u32, (SIZE*IMAGE_SCALE) as u32);

    let mut n = 0;

    let mut r = (0..NCOLLS).collect::<Vec<i32>>();
    r.shuffle(&mut rng);
    println!("building update pattern");
    let mut i = 0;
    for frame in r {
        if i%(NCOLLS/10) == 0 {
            println!("{} / {} points done", i, NCOLLS);
        }
        i+=1;
        let angle = (frame as f64) / (NCOLLS as f64) * PI * 2.0;
        let normal1 = Point::normal(true, angle);
        let normal2 = Point::normal(false, angle);
        
        let best_choice = (0..(1<<10 as i32)).into_par_iter().map(|index_encoded| {
            let index1 = index_encoded >> 5;
            let index2 = index_encoded & 0b11111;
            let mut best_choice_local: Best = Best {
                index1: 0,
                index2:0, 
                pattern: [false, false, false, false], 
                pts1: vec![], 
                pts2: vec![] , 
                normal1: Point::new(0.0, 0.0, 0.0), 
                normal2: Point::new(0.0, 0.0, 0.0)
            };
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
                    let normal = if ptgroup == &&pts1 {&normal1} else {&normal2};
                    if ptgroup.iter().all(|p| p.x == 0.0 && p.y == 0.0) {
                        continue;
                    }
                    if pts.len() == 0 {
                        score += 1000.0; //first frame, dont care what happens
                        continue;
                    }
                    if ptgroup.len() == 1 {
                        let pt = ptgroup[0];
                        score += pt.nearest_point(&pts, normal, &normals).powf(2.0) * (dist_f + pt.dist_to_origin().powf(1.2));
                        //println!("base {}, origin {}", pt.nearest_point(&pts).powf(2.0), pt.dist_to_origin()*10.0);
                    }
                    else {
                        score += ptgroup.iter().map(|p| p.nearest_point(&pts, normal, &normals).powf(2.0) * (dist_f + p.dist_to_origin().powf(1.2))).sum::<f64>() * 0.9;
                    }

                }

                if score > max_score_local {
                    max_score_local = score;
                    best_choice_local = Best { 
                        index1, 
                        index2, 
                        pattern: [activation_pattern[0], activation_pattern[1], activation_pattern[2], activation_pattern[3]], 
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
        let Best {
            index1: _index1,
            index2: _index2, 
            pattern, 
            pts1: new_pts1, 
            pts2: new_pts2, 
            normal1: new_normal1,
            normal2: new_normal2 
        } = best_choice;

        n += pattern[0] as i32 + pattern[1] as i32 + pattern[2] as i32 + pattern[3] as i32;

        for pt in new_pts1.iter().chain(new_pts2.iter()) {
            pts.push(*pt);
            for i in 0..64 {
                let mut shifted = pt.clone();
                shifted.z += i as f64;
                all_pts.push(shifted)
            }
            normals.push(if new_pts1.contains(pt) {new_normal1} else {new_normal2});
            let img_x = pt.x * (IMAGE_SCALE as f64) + ((SIZE * IMAGE_SCALE) as f64) / 2.0;
            let img_y = pt.y * (IMAGE_SCALE as f64) + ((SIZE * IMAGE_SCALE) as f64) / 2.0;
            let color = if new_pts1.contains(pt) {Rgba([255, 0, 0, 255])} else {Rgba([0, 255, 0, 255])};
            draw_filled_circle_mut(&mut img, (img_x as i32, img_y as i32), 2, color);
        }
    }

    let mut sum: f64 = 0.0;

    for i in 0..pts.len() {
        let other_pts = [&pts[..i], &pts[i+1..]].concat();
        sum += pts[i].nearest_point(&other_pts, &normals[i], &normals);
        // println!("{}", pts[i].nearest_point(&other_pts, &normals[i], &normals));
    }
    let avg = sum / pts.len() as f64;
    println!("Average: {}", avg * PITCH as f64);

    let dither_ranks = generate_dither(&all_pts);

    println!("num of points: {}", pts.len());

    save_img(&img, "output.png").expect("Unable to save image");
    write_to_file("output.txt", &best_choices, &dither_ranks).expect("Unable to write to file");
    write_pointcloud_to_file("pointcloud.ply", &pts, &normals).expect("Unable to write point cloud to file");
    println!("{n}");
    let duration = start.elapsed();
    println!("Time elapsed: {:?}", duration);
}
