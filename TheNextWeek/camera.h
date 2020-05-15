#ifndef CAMERAH
#define CAMERAH

#include "ray.h"
#include "helper.h"


vec3 random_in_unit_disk() {
    vec3 p;
    do {
        p = 2.0*vec3(drand48(),drand48(),0) - vec3(1,1,0);
    } while (dot(p,p) >= 1.0);
    return p;
}

class camera {
    public:
		camera(vec3 lookfrom, vec3 lookat, float cam_angle, float vfov, float aspect, float aperture, float focus_dist, float t0, float t1) { // vfov is top to bottom in degrees
            lens_radius = aperture / 2;
            float theta = vfov*PI / 180;
            float half_height = tan(theta / 2);
            float half_width = aspect * half_height;
            origin = lookfrom;
			vec3 viewup((tan(cam_angle*PI / 180)), 1, 0);
            w = unit_vector(lookfrom - lookat);
			u = unit_vector(cross(viewup, w));
            v = cross(w, u);
			time0 = t0;
			time1 = t1;

            lower_left_corner = origin  - half_width*focus_dist*u - half_height*focus_dist*v - focus_dist*w;
            horizontal = 2*half_width*focus_dist*u;
            vertical = 2*half_height*focus_dist*v;
        }
        ray get_ray(float s, float t) {
            vec3 rd = lens_radius * random_in_unit_disk(); //模拟光圈的作用 在光圈设定的立方体中随机生成一个向量 作用于光线的空间位置
            vec3 offset = u * rd.x() + v * rd.y();
			float time = time0 + drand48() * (time1 - time0);//在快门时间内 随机取一个时间点
            return ray(origin + offset, lower_left_corner + s*horizontal + t*vertical - origin - offset, time);
        }

        vec3 origin;
        vec3 lower_left_corner;
        vec3 horizontal;
        vec3 vertical;
        vec3 u, v, w;
        float lens_radius;
		float time0, time1;
};
#endif




