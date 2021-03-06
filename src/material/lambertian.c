#include "material.h"
#include "scene.h"
#include <math.h>

const t_material_vt
	*lambertian_vt(void)
{
	static const t_material_vt	vt = {
		lambertian_scatter,
		lambertian_destroy
	};

	return (&vt);
}

/* TODO: maybe use random_hvec instead */
int
	lambertian_scatter(t_material *mat, t_ray in, t_hit *hit, t_scatter *scatter, t_thread_ctx *ctx)
{
	t_lambertian	*lambertian;
	t_vec			dir;

	(void) in;
	lambertian = (t_lambertian *) mat;
	scatter->attenuation = lambertian->albedo->vt->sample(lambertian->albedo, hit->uv);
	scatter->scattered.pos = hit->pos;
	dir = vec_add(hit->local_normal, rt_random_svec(&ctx->seed));
	if (vec_mag(dir) < 0.001)
		dir = hit->local_normal;
	scatter->scattered.dir = vec_norm(dir);
	scatter->emittance = vec(0, 0, 0, 0);
	return (1);
}

void
	lambertian_destroy(t_material *mat)
{
	t_lambertian	*lambertian;

	lambertian = (t_lambertian *) mat;
	lambertian->albedo->vt->destroy(lambertian->albedo);
	rt_free(mat);
}
