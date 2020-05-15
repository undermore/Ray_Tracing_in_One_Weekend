#include <fstream>
#include <time.h>
#include "sphere.h"
#include "moving_sphere.h"
#include "hitable.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "box.h"
#include "constant_medium.h"
#include "helper.h"
#include "texture.h"
#include "translate.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define SAMPLES	100  //每个像素的采样数量 像素最终颜色=采样均值 相当于长时间曝光降噪
#define USEING_BVH
//#define HIGH_RESOLUTION
//#define PPM

texture *pertext_1 = new noise_texture(1.0);
texture *pertext_2 = new noise_texture(4.0);
texture *pertext_3 = new noise_texture(0.2);
texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));
int ix, iy, n;
unsigned char *tex_data = stbi_load("timg.jpg", &ix, &iy, &n, STBI_rgb);
material *emat = new lambertian(new image_texture(tex_data, ix, iy));
material *dim_light = new diffuse_light(new constant_texture(vec3(4, 4, 4)));
material *bright_light = new diffuse_light(new constant_texture(vec3(10, 10, 10)));
material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
material *blue = new lambertian(new constant_texture(vec3(0.1, 0.1, 0.9)));

vec3 lookfrom(0, 0, 20);//摄像机位置
vec3 lookat(0, 0, 0);//镜头朝向
bool SunLight = true;
float dist_to_focus = 10;// (lookfrom - lookat).length();//焦距
float aperture = 0.01;//光圈
float scene_angle = 0;//场景旋转角度 以w为轴旋转 范围 -90～90
float view_angle = 20;//field of view 镜头视角 加大视角囊括更大场景 相当于拉远镜头
std::string Out_Name = "";


vec3 color(const ray& r, hitable *world, int depth, bool sunlight) {
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
		vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return emitted + attenuation*color(scattered, world, depth + 1, sunlight);
        }
        else {
			return emitted;
        }
    }
    else {
		if (sunlight)
		{
			//线性插值
			vec3 unit_direction = unit_vector(r.direction());
			float t = 0.5*(unit_direction.y() + 1.0);
			return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);//天蓝
		}
		else
		//关灯
		return vec3(0, 0, 0);
    }
}

hitable *Final() {
	lookfrom = vec3(478, 278, -600);
	lookat = vec3(278,278,0);
	SunLight = false;
	view_angle = 40;
	Out_Name = "Final";
	int nb = 20;
	hitable **list = new hitable*[30];
	hitable **boxlist = new hitable*[10000];
	hitable **boxlist2 = new hitable*[10000];
	material *ground = new lambertian(new constant_texture(vec3(0.48, 0.83, 0.53)));
	int b = 0;
	for (int i = 0; i < nb; i++) {
		for (int j = 0; j < nb; j++) {
			float w = 100;
			float x0 = -1000 + i*w;
			float z0 = -1000 + j*w;
			float y0 = 0;
			float x1 = x0 + w;
			float y1 = 100 * (drand48() + 0.01);
			float z1 = z0 + w;
			boxlist[b++] = new box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
		}
	}
	int l = 0;
	list[l++] = new bvh_node(boxlist, b, 0, 1);
	list[l++] = new xz_rect(123, 423, 147, 412, 554, bright_light);
	vec3 center(400, 400, 200);
	list[l++] = new moving_sphere(center, center + vec3(30, 0, 0), 0, 1, 50, new lambertian(new constant_texture(vec3(0.7, 0.3, 0.1))));//移动球
	list[l++] = new sphere(vec3(260, 150, 45), 50, new dielectric(1.5));//玻璃球
	list[l++] = new sphere(vec3(0, 150, 145), 50, new metal(vec3(0.8, 0.8, 0.9), 10.0));//金属球

	hitable *boundary = new sphere(vec3(360, 150, 145), 70, new dielectric(1.5));
	list[l++] = boundary;
	list[l++] = new constant_medium(boundary, 0.2, new constant_texture(vec3(0.2, 0.4, 0.9)));//蓝色球

	//boundary = new sphere(vec3(0, 0, 0), 5000, new dielectric(1.5));
	//list[l++] = new constant_medium(boundary, 0.0001, new constant_texture(vec3(1.0, 1.0, 1.0)));

	list[l++] = new sphere(vec3(400, 200, 400), 100, emat);//贴图球
	list[l++] = new sphere(vec3(220, 280, 300), 80, new lambertian(pertext_2));//花纹球

	//球立方
	int ns = 1000;
	for (int j = 0; j < ns; j++) {
		boxlist2[j] = new sphere(vec3(165 * drand48(), 165 * drand48(), 165 * drand48()), 10, white);
	}
	list[l++] = new translate(new rotate_y(new bvh_node(boxlist2, ns, 0.0, 1.0), 15), vec3(-100, 270, 395));

#ifdef USEING_BVH
	return new bvh_node(list, l, 0.0, 1.0);
#else
	return new hitable_list(list, l);
#endif

}

hitable *Cornell_Box() {

	lookfrom = vec3(278, 278, -800);
	lookat = vec3(278,278,0);
	SunLight = false;
	view_angle = 40;
	Out_Name = "CornellBox";
	hitable **list = new hitable*[30];
	int i = 0;
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, red));//右侧墙 因为摄像机z坐标<0
	list[i++] = new yz_rect(0, 555, 0, 555, 0, green);//左墙
	list[i++] = new xz_rect(123, 423, 147, 412, 554, bright_light);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));//天棚
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);//地面
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, blue));
	/*
	hitable *b1 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
	hitable *b2 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));
	list[i++] = new constant_medium(b1, 0.01, new constant_texture(vec3(1.0, 1.0, 1.0)));
	list[i++] = new constant_medium(b2, 0.01, new constant_texture(vec3(0.0, 0.0, 0.0)));
	*/
	hitable *boundary1 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), new dielectric(1.5)), -18), vec3(130, 0, 65));
	list[i++] = boundary1;
	list[i++] = new constant_medium(boundary1, 0.2, new constant_texture(vec3(0.9, 0.9, 0.9)));

	hitable *boundary2 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), new dielectric(1.5)), 15), vec3(265, 0, 295));
	list[i++] = boundary2;
	list[i++] = new constant_medium(boundary2, 0.2, new constant_texture(vec3(0.9, 0.9, 0.9)));

#ifdef USEING_BVH
	return new bvh_node(list, i, 0.0, 1.0);
#else
	return new hitable_list(list, i);
#endif
}


hitable *Two_Spheres() {
	lookfrom = vec3(0, 0, 50);
	lookat = vec3(0, 0, 0);
	SunLight = true;
	view_angle = 60;
	Out_Name = "TwoSpheres";
	hitable **list = new hitable*[2];
	list[0] = new sphere(vec3(0, -10, 0), 10, new lambertian(pertext_1));
	list[1] = new sphere(vec3(0, 10, 0), 10, emat);

#ifdef USEING_BVH
	return new bvh_node(list, 2, 0.0, 1.0);
#else
	return new hitable_list(list, 2);
#endif

}

hitable *Light() {
	lookfrom = vec3(10, 2, 5);
	lookat = vec3(2, 2, 0);
	SunLight = false;
	view_angle = 60;
	Out_Name = "Light";
	hitable **list = new hitable*[4];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext_2));
	list[1] = new sphere(vec3(2, 2, 1), 2, new metal(vec3(0.1, 0.1, 0.8), 10.0));
	list[2] = new xy_rect(2, 6, 1, 3, -2, dim_light);

#ifdef USEING_BVH
	return new bvh_node(list, 3, 0.0, 1.0);
#else
	return new hitable_list(list, 3);
#endif

}

hitable *Random_Scene() {
	lookfrom = vec3(13, 2, 3);
	lookat = vec3(0,0,0);
	SunLight = true;
	Out_Name = "RandomScene";
	int n = 50000;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			float choose_mat = drand48();
			vec3 center(a + 0.9*drand48(), 0.2, b + 0.9*drand48());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new sphere(center, 0.2, new lambertian(new constant_texture(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48()))) );
					//list[i++] = new moving_sphere(center, center + vec3(0, 0.5*drand48(), 0), 0.0, 1.0, 0.2, new lambertian(new constant_texture(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48()))));
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(center, 0.2,
						new metal(vec3(0.5*(1 + drand48()), 0.5*(1 + drand48()), 0.5*(1 + drand48())), 0.5*drand48()));
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

#ifdef USEING_BVH
	return new bvh_node(list, i, 0.0, 1.0);
#else
	return new hitable_list(list,i);
#endif
}

int main() {
#ifdef HIGH_RESOLUTION
	int nx = 800;
	int ny = 800;
#else
	int nx = 400;
	int ny = 400;
#endif
    int ns = SAMPLES;
	srand48(time(0));
#ifdef PPM
	freopen("out.ppm", "w", stdout);
    std::cout << "P3\n" << nx << " " << ny << "\n255\n";
#endif

	/*
	hitable *list[5];
    list[0] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));//地面
	list[1] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.5, 0.5, 0.5)));//漫反射球
	//list[2] = new moving_sphere(vec3(1, 0, -1), vec3(1, 2, -1), 0.0, 2.0, 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));//金属球
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));//金属球
    list[3] = new sphere(vec3(-1,0,-1), 0.5, new dielectric(1.5));//玻璃球
    list[4] = new sphere(vec3(-1,0,-1), -0.475, new dielectric(1.5));//玻璃球重叠 构成空心玻璃球
    hitable *world = new hitable_list(list,5);
	*/
	hitable *world = new hitable_list();
    //world = Random_Scene();
	//world = Light();
	//world = Two_Spheres();
	//world = Cornell_Box();
	world = Final();


	camera cam(lookfrom, lookat, scene_angle, view_angle, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 0.2);

	unsigned char *img_data = new unsigned char[ny * nx * STBI_rgb * sizeof(unsigned char)];

	for (int j = ny - 1, k = 0; j >= 0; j--) {
        for (int i = 0; i < nx; i++, k += 3) {
            vec3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                ray r = cam.get_ray(u, v);
				col += color(r, world, 0, SunLight);
            }
            col /= float(ns);
            col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
			col[0] = col[0] > 1.0 ? 1.0 : col[0];
			col[1] = col[1] > 1.0 ? 1.0 : col[1];
			col[2] = col[2] > 1.0 ? 1.0 : col[2];
			img_data[k] = (unsigned char)255.99*col[0];
			img_data[k+1] = (unsigned char)255.99*col[1];
			img_data[k+2] = (unsigned char)255.99*col[2];
#ifdef PPM
			std::cout << (int)img_data[k] << " " << (int)img_data[k + 1] << " " << (int)img_data[k + 2] << "\n";
#endif
        }
#ifndef PPM
		float progress = floor(100.0*((float)(ny - j) / (float)ny));
		std::cout<<progress<< "%"<<"\r";
#endif
    }
	char *name = new char[100];
#ifdef USEING_BVH
	sprintf(name, "%s_Smaple_%d_BVH.jpg", Out_Name.c_str(), ns);
#else
	sprintf(name, "%s_Smaple_%d_no_BVH.jpg", Out_Name.c_str(), ns);
#endif
	stbi_write_jpg(name, nx, ny, STBI_rgb, img_data, 100);

}

