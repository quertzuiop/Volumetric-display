import cv2
import numpy as np
from scipy.spatial import KDTree
from itertools import product
import random
from tqdm import tqdm
import cProfile

ncols = 600
height,width = 64,64
pitch=2.5
panel2_offset = 17/pitch # in mm / pitch in mm
img_scale = 12

def get_point_pos(index, side2, angle, offset=None):
    panel_center_dist = (index + (side2 * width/2) - width/2 + 0.5)
    if offset is None:
        x = panel_center_dist * np.cos(angle)
        y = panel_center_dist * np.sin(angle)
    else:
        x = -offset * np.sin(angle) - panel_center_dist * np.cos(angle)
        y = offset * np.cos(angle) - panel_center_dist * np.sin(angle)
    return x, y

def nearest_neighbor_distance(pts, pt, tree):
    if len(pts) == 0:
        return 0
    dists, _ = tree.query([pt], k=2)  # k=2 to include the point itself (distance 0)
    
    return np.average(dists[0][:1])

pts_display1 = np.array([])
pts_display2 = np.array([])
img = np.zeros((height*img_scale, width*img_scale, 3), dtype=np.uint8)

r = list(range(ncols))
n=0
random.shuffle(r)

for frame in tqdm(r):
    angle = frame / ncols * 2 * np.pi
    max_score = float('-inf')
    best_choice = None
    
    for index in range(width//2):
        for activation_pattern in product(range(2), repeat=4):
            if 1 not in activation_pattern: continue
            pts1, pts2 = [], []
            for i, active in enumerate(activation_pattern):
                if i < 2 and active:
                    pts1 += [get_point_pos(index, i%2, angle)]
                elif i >= 2 and active:
                    pts2 += [get_point_pos(index, i%2, angle, offset=panel2_offset)]
            score = 0
            for ptgroup in [pts1, pts2]:
                pts = pts_display1 if ptgroup is pts1 else pts_display2
                if len(ptgroup) == 0: continue
                if len (pts) == 0:
                    score += 1000  # Large constant for first points
                    continue
                tree = KDTree(pts)
                if len(ptgroup) == 1:
                    score += nearest_neighbor_distance(pts, ptgroup[0], tree)
                else:
                    score += (nearest_neighbor_distance(pts, ptgroup[0], tree) + nearest_neighbor_distance(pts, ptgroup[1], tree))* 0.65
            
            if score > max_score:
                max_score = score
                best_choice = (index, activation_pattern, pts1, pts2)
                
    index, activation_pattern, pts1, pts2 = best_choice
    n+=sum(activation_pattern)
    if frame % 100 == 0:
        print(" ", n, " ") #spaces to prevent tqdm overwriting
    for pt in pts1:
        pts_display1 = np.append(pts_display1, [pt], axis=0) if pts_display1.size else np.array([pt])
        x = int(pt[0]*img_scale + width*img_scale/2)
        y = int(pt[1]*img_scale + height*img_scale/2)
        img = cv2.circle(img, (x, y), 1, (0, 0, 255), -1)
    for pt in pts2:
        pts_display2 = np.append(pts_display2, [pt], axis=0) if pts_display2.size else np.array([pt])
        x = int(pt[0]*img_scale + width*img_scale/2)
        y = int(pt[1]*img_scale + height*img_scale/2)
        img = cv2.circle(img, (x, y), 1, (0, 255, 0), -1)
    #print(activation_pattern)
    #if left_on:
    #    pts = np.append(pts, [pt_l], axis=0)
    #if right_on:
    #    pts = np.append(pts, [pt_r], axis=0)
    cv2.imshow('Pattern Visualization', img)
    cv2.waitKey(1)