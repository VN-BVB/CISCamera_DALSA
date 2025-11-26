import glob, os
import numpy as np
import cv2
from scipy.optimize import curve_fit, least_squares

def to_homogeneous(points):
    if points.shape[1] == 2 or points.shape[1] == 3:
        ones = np.ones((points.shape[0], 1))
        return np.hstack((points, ones))
    return points

def distort(k, normalized_proj):
    x, y = normalized_proj[:, 0], normalized_proj[:, 1]
    r = x ** 2 + y ** 2

    k1, h1, h2, s1, s2 = k

    deltaX = k1*x*r + h1*(3*x*x + y*y) + 2*h2*x*y + s1*r
    deltaY = k1*y*r + 2*h1*x*y + h2*(x*x + 3*y*y) + s2*r

    x_prime = x + deltaX
    y_prime = y + deltaY

    distorted_proj = np.hstack((x_prime[:, None], y_prime[:, None]))
    return to_homogeneous(distorted_proj)

def refine_params_with_distortion_external_only(points_world, points_pixel, K, coff_dis, v_rot, v_trans):
    points_pixel = np.array(points_pixel)
    points_world = np.array(points_world)
    print("\n-- K (camera matrix) --")
    print(K)
    coff_dis_opt = np.array(coff_dis)
    # convert to homogeneous (x,y,1)
    points_world = np.hstack([points_world, np.ones((points_world.shape[0], 1))])
    # add batch dimension
    points_world = points_world.reshape(1, points_world.shape[0], 3)
    points_pixel = points_pixel.reshape(1, points_pixel.shape[0], 2)
    v_rot = v_rot.reshape(1, 3)
    v_trans = np.array(v_trans)[:2].reshape(1, 2)
    n_views = len(v_rot)
    print("===== Inputs =====")
    print("-- points_world --")
    print("type:", type(points_world))
    print("shape:", points_world.shape)

    print("\n-- points_pixel --")
    print("type:", type(points_pixel))
    print("shape:", points_pixel.shape)

    print("\n-- K (camera matrix) --")
    print("type:", type(K))
    print("shape:", K.shape)

    print("\n-- coff_dis (distortion coefficients) --")
    print("type:", type(coff_dis))
    print("shape:", coff_dis.shape)

    print("\n-- v_rot --")
    print("type:", type(v_rot))
    print("shape:", v_rot.shape)

    print("\n-- v_trans --")
    print("type:", type(v_trans))
    print("shape:", v_trans.shape)
    print("==================\n")
    def compute_reproj_loss(v_rot_list, v_trans_list):
        total_err = 0.0
        total_points = 0
        for i in range(n_views):
            world_points = points_world[i].reshape(-1, 3)
            world_points[:, 2] = 1
            pixel_gt = points_pixel[i].reshape(-1, 2)

            rot_mat, _ = cv2.Rodrigues(np.array(v_rot_list[i], dtype=np.float64).reshape(3))
            R2 = rot_mat[:2, :2]
            t2 = np.array(v_trans_list[i], dtype=np.float64).reshape(2)
            for pt_idx in range(world_points.shape[0]):
                xy = world_points[pt_idx, :2]
                cam_xy = R2 @ xy + t2
                camPt = cam_xy.reshape(1, 2)
                distortedH = distort(coff_dis_opt, camPt)
                uvw = K @ distortedH[0].T
                uv_hat = np.array([uvw[0] / uvw[2], uvw[1] / uvw[2]])
                total_err += np.linalg.norm(uv_hat - pixel_gt[pt_idx])
                total_points += 1
        mean_loss = total_err / total_points
        return mean_loss
    initial_loss = compute_reproj_loss(v_rot, v_trans)
    print(f"Initial reprojection error: {initial_loss:.6f}")
    packed_params = []
    for i in range(n_views):
        packed_params.extend(list(v_rot[i]))
        packed_params.extend(list(v_trans[i]))
    packed_params = np.array(packed_params, dtype=np.float64)
    def project_external_only(x_data, *params):
        y_pre_list = []
        for i in range(n_views):
            idx = i * 5
            rt = params[idx: idx + 5]
            rot_vec = np.array(rt[:3], dtype=np.float64).reshape(3)
            trans_vec = np.array(rt[3:], dtype=np.float64).reshape(2)

            world_points = np.array(x_data[i]).reshape(-1, 3)
            world_points[:, 2] = 1

            rot_mat, _ = cv2.Rodrigues(rot_vec)
            rt_matri = np.eye(3)
            rt_matri[:2, :2] = rot_mat[:2, :2]
            rt_matri[:2, 2] = trans_vec

            y_normalized = (rt_matri @ world_points.T).T
            y_distorted = distort(coff_dis_opt, y_normalized)
            y_pixel = (K @ y_distorted.T).T
            y_pre_list.append(y_pixel[:, :2])
        return np.array(y_pre_list).reshape(-1)
    popt, _ = curve_fit(
        project_external_only,
        points_world,
        points_pixel.reshape(-1),
        p0=packed_params,
        maxfev=10000000
    )
    v_rot_refined = []
    v_trans_refined = []
    for i in range(n_views):
        v_rot_refined.append(popt[i * 5: i * 5 + 3])
        v_trans_refined.append(popt[i * 5 + 3: (i + 1) * 5])
    v_rot_refined = np.array(v_rot_refined)
    v_trans_refined = np.array(v_trans_refined)
    final_loss = compute_reproj_loss(v_rot_refined, v_trans_refined)
    print(f"Final reprojection error: {final_loss:.6f}")
    print(f"Error improvement: {initial_loss - final_loss:.6f}")
    return final_loss, v_rot_refined, v_trans_refined