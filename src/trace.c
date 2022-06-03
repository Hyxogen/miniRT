#include "rt.h"
#include "tree.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*
static int
	trace_hit(t_scene *scene, t_ray ray, t_hit *hit)
{
	t_hit		tmp;
	size_t		i;
	t_entity	*ent;

	i = 0;
	hit->t = RT_RAY_LENGTH;
	while (i < scene->count)
	{
		ent = scene->entities[i];
		if (ent->vt->hit != NULL && ent->vt->hit(ent, ray, &tmp, 0.001))
			if (tmp.t < hit->t)
				*hit = tmp;
		i += 1;
	}
	hit->local_normal = hit->normal;
	if (vec_dot(ray.dir, hit->normal) >= 0)
		hit->local_normal = vec_neg(hit->normal);
	return (hit->t < RT_RAY_LENGTH);
}
*/

/*
static t_vec
	trace_ray(t_thread_ctx *ctx, t_scene *scene, t_ray ray, int depth)
{
	t_hit	hit;
	t_ray	new_ray;
	FLOAT	p;
	FLOAT	cos_theta;
	t_vec	BRDF;
	t_vec	incoming;

	// printf("here ");
	if (depth <= 0)
		return (vec(0.0, 0.0, 0.0, 0.0));
	if (trace_hit(scene, ray, &hit))
	{
		p = 1.0 / (RT_2PI);
//		BRDF = vec_scale(vec(0.5, 0.5, 0.5, 1.0), 1.0 / RT_PI);
		BRDF = vec_scale(vec(5.0, 5.0, 5.0, 1.0), 1);
		new_ray.pos = vec_add(hit.pos, vec_scale(hit.normal, 0.01));
		new_ray.dir = rt_random_hvec(&ctx->seed, hit.normal);
		cos_theta = vec_dot(new_ray.dir, hit.normal);
		incoming = trace_ray(ctx, scene, new_ray, depth - 1);
		return (vec_clamp(vec_add(vec_scale(hit.color, 0.5), vec_scale(color_mul(BRDF, incoming), cos_theta / p)), 0.0, 1.0));
	}
	return (vec(0.0, 0.0, 0.0, 0.0));
}
*/

static t_vec
	trace_lights(t_scene *scene, t_ray ray, FLOAT min, FLOAT max)
{
	FLOAT	near_dist;
	FLOAT	dist;
	t_vec	light_pos;
	FLOAT	const_att;
	FLOAT	linear_att;
	FLOAT	quad_att;
	t_ray	light_ray;
	t_hit	obj_hit;
	FLOAT	t;
 
	if (scene->main_light == NULL)
		return (vec(0, 0, 0, 0));
	light_pos = scene->main_light->pos;

	const_att = 0.0;
	linear_att = 0.0;
	quad_att = 10.0;

//	near_dist = vec_mag(
//			vec_sub(
//				vec_sub(light_pos, ray.pos),
//				vec_scale(ray.dir, vec_dot(
//						vec_sub(light_pos, ray.pos), ray.dir))));
	/*
	if (near_dist > 1) //TODO remove magic number
		return (vec(0, 0, 0, 0));
	*/
//	light_ray.pos = hit->pos;
	t = vec_dot(vec_sub(light_pos, ray.pos), ray.dir);
	light_ray.pos = vec_add(ray.pos, vec_scale(ray.dir, t));
	near_dist = vec_mag(vec_sub(light_ray.pos, light_pos));
	light_ray.dir = vec_norm(vec_sub(light_pos, light_ray.pos));
//	if (vec_dot(light_ray.dir, hit->normal) >= 0)
//		return (vec(0, 0, 0, 0));
//	light_ray.pos = vec_add(light_ray.pos, vec_scale(light_ray.dir, 0.0001));
	if (!tree_hit(scene->tree, light_ray, &obj_hit, HUGE_VAL))
		obj_hit.t = HUGE_VAL;
	dist = near_dist;
	//dist = vec_mag(vec_sub(hit->pos, light_pos));
	if (obj_hit.t < dist || t < min || t > max)
		return (vec(0, 0, 0, 0));
	return (vec_scale(scene->main_light->color, 1.0 / (const_att + (linear_att * dist) + (quad_att * dist * dist))));
}

/*
static t_vec
	trace_ray(t_thread_ctx *ctx, t_scene *scene, t_ray ray, int depth)
{
	FLOAT		t;
	t_hit		hit;
	t_scatter	scatter;
	t_vec		light_col;

	if (depth == 0)
		return (vec(0, 0, 0, 0));
	if (trace_hit(scene, ray, &hit))
	{
		light_col = trace_lights(scene, ray, 0.001, hit.t);
		if (hit.mat->vt->scatter(hit.mat, ray, &hit, &scatter, ctx))
			return (vec_add(light_col, color_mul(scatter.attenuation, trace_ray(ctx, scene, scatter.scattered, depth - 1))));
	}
	light_col = trace_lights(scene, ray, 0.001, RT_RAY_LENGTH);
	return (light_col);
	t = 0.5 * (ray.dir.v[Z] + 1.0);
	return (vec_add(vec_scale(vec(1, 1, 1, 0), 1.0 - t), vec_scale(vec(0.5, 0.7, 1.0, 0.0), t)));
}
*/

static t_vec
	trace_ray(t_thread_ctx *ctx, t_scene *scene, t_ray ray, int depth)
{
	t_hit		hit;
	t_scatter	scatter;

	if (depth == 0)
		return (vec(0, 0, 0, 0));
	if (tree_hit(scene->tree, ray, &hit, HUGE_VAL))
	{
		// TODO: use the BRDF stuff
		if (hit.mat->vt->scatter(hit.mat, ray, &hit, &scatter, ctx) && vec_mag(scatter.attenuation) > 0)
			return (vec_add(scatter.emittance, color_mul(scatter.attenuation, trace_ray(ctx, scene, scatter.scattered, depth - 1))));
		return (scatter.emittance);
	}
	if (scene->ambient_light != NULL)
		return (vec_scale(scene->ambient_light->color, scene->ambient_light->ratio));
	return (vec(0, 0, 0, 0));
}

void
	trace_debug(t_rt_state *state, int x, int y)
{
	t_ray				ray;
	t_hit				hit;
	t_scatter			scatter;
	static t_thread_ctx	ctx = { 7549087012 };

	ray = project_ray(state, x, y);
	mutex_lock(&state->mtx);
	state->dbg_line[0] = ray.pos;
	state->dbg_line_size = 1;
	state->dbg_norm_size = 0;
	while (state->dbg_line_size < RT_MAX_DEPTH + 1)
	{
		if (!tree_hit(state->scene.tree, ray, &hit, HUGE_VAL))
		{
			state->dbg_line[state->dbg_line_size] = vec_add(ray.pos, vec_scale(ray.dir, RT_RAY_LENGTH));
			state->dbg_line_size += 1;
			break ;
		}
		state->dbg_norm[state->dbg_norm_size][0] = hit.pos;
		state->dbg_norm[state->dbg_norm_size][1] = vec_add(hit.pos, vec_scale(hit.normal, 0.1));
		state->dbg_norm_size += 1;
		if (!hit.mat->vt->scatter(hit.mat, ray, &hit, &scatter, &ctx))
		{
			state->dbg_line[state->dbg_line_size] = hit.pos;
			state->dbg_line_size += 1;
			break ;
		}
		ray = scatter.scattered;
		state->dbg_line[state->dbg_line_size] = ray.pos;
		state->dbg_line_size += 1;
	}
	thread_reset(state);
	mutex_unlock(&state->mtx);
}

FLOAT
	trace_line(t_ray ray, t_vec src, t_vec dst)
{
	t_vec	dir;
	t_vec	cross;
	t_vec	c1;
	t_vec	c2;
	t_vec	n1;
	t_vec	n2;
	FLOAT	t1;
	FLOAT	t2;
		
	dir = vec_norm(vec_sub(dst, src));
	cross = vec_cross(ray.dir, dir);
	if (vec_mag(cross) != 0)
	{
		n2 = vec_cross(dir, cross);
		t1 = vec_dot(vec_sub(src, ray.pos), n2) / vec_dot(ray.dir, n2);
		c1 = vec_add(ray.pos, vec_scale(ray.dir, t1));
		n1 = vec_cross(ray.dir, cross);
		t2 = vec_dot(vec_sub(ray.pos, src), n1) / vec_dot(dir, n1);
		c2 = vec_add(src, vec_scale(dir, t2));
		if (t2 > 0 && t2 < vec_mag(vec_sub(dst, src)) && t1 > 0)
			return (vec_mag(vec_sub(c1, c2)));
	}
	return (HUGE_VAL);
}

t_vec
	trace(t_thread_ctx *ctx, t_rt_state *state, int x, int y)
{
	t_ray	ray;
	size_t	i;
	FLOAT	min_dist;
	FLOAT	dist;
	t_vec	color;

	ray = project_ray(state,
			x + rt_random_float(&ctx->seed), y + rt_random_float(&ctx->seed));
	min_dist = HUGE_VAL;
	i = 0;
	while (i < state->dbg_norm_size)
	{
		dist = trace_line(ray, state->dbg_norm[i][0], state->dbg_norm[i][1]);
		if (dist < min_dist)
		{
			color = vec(0, 0, 1, 0);
			min_dist = dist;
		}
		i += 1;
	}
	i = 0;
	while (i < state->dbg_line_size && i < state->dbg_line_size - 1)
	{
		dist = trace_line(ray, state->dbg_line[i], state->dbg_line[i + 1]);
		if (dist < min_dist)
		{
			color = vec(1, 0, 0, 0);
			min_dist = dist;
		}
		i += 1;
	}
	if (min_dist < 0.0025)
		return (color);
	else
		return (trace_ray(ctx, &state->scene, ray, RT_MAX_DEPTH));
}
