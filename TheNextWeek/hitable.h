#ifndef HITABLEH
#define HITABLEH

#include "ray.h"
#include "aabb.h"


class material;


struct hit_record
{
    float t;  //光线与物体交点的参数t
    vec3 p;//交点
    vec3 normal;//交点的法向量
    material *mat_ptr;
	float u, v;//物体表面坐标
};

class hitable  {
    public:
        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
		virtual bool bounding_box(float t0, float t1, aabb& box) const = 0;
};


class hitable_list : public hitable  {
public:
	hitable_list() {}
	hitable_list(hitable **l, int n) { list = l; list_size = n; }
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;
	hitable **list;
	int list_size;
};

bool hitable_list::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	hit_record temp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;
	for (int i = 0; i < list_size; i++) {
		if (list[i]->hit(r, t_min, closest_so_far, temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}
	return hit_anything;
}

bool hitable_list::bounding_box(float t0, float t1, aabb& box) const{
	if (list_size < 1){
		return false;
	}
	aabb temp_box;
	bool first_true = list[0]->bounding_box(t0, t1, temp_box);
	if (!first_true)
		return false;
	else
		box = temp_box;
	for (int i = 1; i < list_size; i++){
		if (list[0]->bounding_box(t0, t1, temp_box)){
			box = surrounding_box(box, temp_box);
		}
		else{
			return false;
		}
	}
	return true;
}

#endif
