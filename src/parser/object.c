#include "parser.h"

#include "util.h"
#include "scene.h"
#include <libft.h>
#include <math.h>


#include <stdio.h>

t_entity
	*rt_triangle(t_scene *scene, const char **line, char **error)
{
	t_triangle	triangle;

	*line = rt_pos(*line, error, &triangle.pos0);
	*line = rt_pos(*line, error, &triangle.pos1);
	*line = rt_pos(*line, error, &triangle.pos2);
	triangle.mat = rt_material(scene, line, error);
	if (*line == NULL)
		return (NULL);
	triangle.base.vt = triangle_vt();
	triangle.normal = vec_norm2(vec_cross(
						
						vec_sub(triangle.pos2, triangle.pos0),
						vec_sub(triangle.pos1, triangle.pos2)));
	triangle.v0 = vec_sub(triangle.pos1, triangle.pos0);
	triangle.v1 = vec_sub(triangle.pos2, triangle.pos0);
	triangle.d00 = vec_dot(triangle.v0, triangle.v0);
	triangle.d01 = vec_dot(triangle.v0, triangle.v1);
		triangle.d11 = vec_dot(triangle.v1, triangle.v1);
	triangle.inv_denom = 1.0 / (triangle.d00 * triangle.d11 - triangle.d01 * triangle.d01);
	return (rt_memdup(&triangle, sizeof(triangle)));
}

t_entity
	*rt_sphere(t_scene *scene, const char **line, char **error)
{
	t_sphere	sphere;

	*line = rt_pos(*line, error, &sphere.pos);
	*line = rt_float(*line, error, &sphere.diameter);
	sphere.mat = rt_material(scene, line, error);
	if (*line == NULL)
		return (NULL);
	sphere.base.vt = sphere_vt();
	sphere.radius = sphere.diameter / 2.0;
	return (rt_memdup(&sphere, sizeof(sphere)));
}

t_entity
	*rt_cone(t_scene *scene, const char **line, char **error)
{
	t_cone	cone;

	*line = rt_pos(*line, error, &cone.pos);
	*line = rt_norm_vec(*line, error, &cone.dir);
	*line = rt_float(*line, error, &cone.angle);
	*line = rt_float(*line, error, &cone.height);
	cone.mat = rt_material(scene, line, error);
	if (*line == NULL)
		return (NULL);
	cone.base.vt = cone_vt();
	cone.costheta2 = pow(cos(cone.angle), 2.0);
	cone.radius = tan(cone.angle) * cone.height;
	return (rt_memdup(&cone, sizeof(cone)));
}

t_entity
	*rt_plane(t_scene *scene, const char **line, char **error)
{
	t_plane		plane;

	*line = rt_pos(*line, error, &plane.pos);
	*line = rt_norm_vec(*line, error, &plane.dir);
	plane.mat = rt_material(scene, line, error);
	if (*line == NULL)
		return (NULL);
	plane.base.vt = plane_vt();
	return (rt_memdup(&plane, sizeof(plane)));
}

t_entity
	*rt_cylinder(t_scene *scene, const char **line, char **error)
{
	t_cylinder	cyl;

	*line = rt_pos(*line, error, &cyl.pos);
	*line = rt_norm_vec(*line, error, &cyl.dir);
	*line = rt_float(*line, error, &cyl.diameter);
	*line = rt_float(*line, error, &cyl.height);
	cyl.mat = rt_material(scene, line, error);
	if (*line == NULL)
		return (NULL);
	cyl.base.vt = cylinder_vt();
	cyl.radius = cyl.diameter / 2.0;
	return (rt_memdup(&cyl, sizeof(cyl)));
}
