#include "work.h"
#include "util.h"
#include "world_impl.h"
#include "parser.h"

#include <ft_printf.h>
#include <stdlib.h>
#include <unistd.h>


#include <stdio.h>

#define GEN_MIN_SCALE 0.2
#define GEN_MIN_ANGLE 0.25
#define GEN_X_SIZE 1
#define GEN_Y_SIZE (320 / 4)
#define GEN_Z_SIZE (180 / 4)
#define GEN_X_RANGE 0
#define GEN_Y_RANGE 0
#define GEN_Z_RANGE 0
#define GEN_SCALE_FACTOR 3.0
#define GEN_SCALE (GEN_SCALE_FACTOR / (GEN_X_SIZE + GEN_Y_SIZE + GEN_Z_SIZE))
#define GEN_SPHERE_CHANCE (0.0 / 3)
#define GEN_CYLINDER_CHANCE (3.0 / 3)
#define GEN_CONE_CHANCE (0.0 / 3)

void
	world_gen_sphere(t_world *world, t_seed *seed, t_vec pos, FLOAT scale)
{
	t_shape_sphere	sphere;

	sphere.base.data = RT_SHAPE_SPHERE;
	sphere.radius = rt_random_float_range(seed, scale * GEN_MIN_SCALE, scale);
	sphere.pos = pos;
	world_add_primitive(world, &sphere, sizeof(sphere));
}

void
	world_gen_cylinder(t_world *world, t_seed *seed, t_vec pos, FLOAT scale)
{
	t_shape_cylinder	cylinder;

	cylinder.base.data = RT_SHAPE_CYLINDER;
	cylinder.cylinder.dir = vec_norm(vec(
				rt_random_float_range(seed, -1, 1),
				rt_random_float_range(seed, -1, 1),
				rt_random_float_range(seed, -1, 1)));
	cylinder.cylinder.radius = rt_random_float_range(seed, scale * GEN_MIN_SCALE, scale);
	cylinder.cylinder.height = rt_random_float_range(seed, scale * GEN_MIN_SCALE * 2, scale * 2);
	cylinder.cylinder.pos = vec_sub(pos, vec_scale(cylinder.cylinder.dir, cylinder.cylinder.height / 2));
	world_add_primitive(world, &cylinder, sizeof(cylinder));
}

void
	world_gen_cone(t_world *world, t_seed *seed, t_vec pos, FLOAT scale)
{
	t_shape_cone	cone;

	cone.base.data = RT_SHAPE_CONE;
	cone.cone.dir = vec_norm(vec(
				rt_random_float_range(seed, -1, 1),
				rt_random_float_range(seed, -1, 1),
				rt_random_float_range(seed, -1, 1)));
	cone.cone.angle = rt_random_float_range(seed, RT_PI / 6 * GEN_MIN_ANGLE, RT_PI / 6);
	cone.cone.height = rt_random_float_range(seed, scale * GEN_MIN_SCALE * 2, scale * 2);
	cone.cone.pos = vec_sub(pos, vec_scale(cone.cone.dir, cone.cone.height / 2));
	world_add_primitive(world, &cone, sizeof(cone));
}

FLOAT
	world_gen_offset(t_seed *seed, int axis)
{
	if (axis == 0 && GEN_X_RANGE != 0)
		return (rt_random_float_range(seed, 0, GEN_X_RANGE));
	if (axis == 1 && GEN_Y_RANGE != 0)
		return (rt_random_float_range(seed, 0, GEN_Y_RANGE));
	if (axis == 2 && GEN_Z_RANGE != 0)
		return (rt_random_float_range(seed, 0, GEN_Z_RANGE));
	return (0);
}

void
	world_gen(t_world *world)
{
	size_t	i;
	t_seed	seed;
	t_vec	pos;
	FLOAT	type;

	seed = 7549087012;
	i = 0;
	while (i < GEN_X_SIZE * GEN_Y_SIZE * GEN_Z_SIZE)
	{
		pos = vec(
			+0.5 + (world_gen_offset(&seed, 0) + i % GEN_X_SIZE) / (FLOAT) GEN_Z_SIZE,
			-(FLOAT) GEN_Y_SIZE / GEN_Z_SIZE / 2 + (world_gen_offset(&seed, 1) + i / GEN_X_SIZE % GEN_Y_SIZE) / (FLOAT) GEN_Z_SIZE,
			-0.5 + (world_gen_offset(&seed, 2) + i / GEN_X_SIZE / GEN_Y_SIZE % GEN_Z_SIZE) / (FLOAT) GEN_Z_SIZE);
		pos = vec_scale(pos, 2);
		type = rt_random_float_range(&seed, 0, 1);
		if (type < GEN_SPHERE_CHANCE)
			world_gen_sphere(world, &seed, pos, GEN_SCALE);
		else
		{
			type -= GEN_SPHERE_CHANCE;
			if (type < GEN_CYLINDER_CHANCE)
				world_gen_cylinder(world, &seed, pos, GEN_SCALE);
			else
			{
				type -= GEN_CYLINDER_CHANCE;
				if (type < GEN_CONE_CHANCE)
					world_gen_cone(world, &seed, pos, GEN_SCALE);
				else
				{
					type -= GEN_CONE_CHANCE;
				}
			}
		}
		i += 1;
	}
	init_camera(world);
}

void
	world_load(t_world *world, const char *filename)
{
	char		*file;
	t_parse_ctx	ctx;
	char		*error;

	file = rt_readfile(filename, &error, NULL);
	if (file == NULL)
	{
		ft_fprintf(STDERR_FILENO, "Error\n%s\n", error);
		rt_free(error);
		exit(EXIT_FAILURE);
	}
	ctx.data = file;
	ctx.filename = filename;
	ctx.line = 1;
	ctx.column = 1;
	rt_world(world, &ctx);
	rt_free(file);
}

#define RT_VERBOSE
void
	dump_tree(t_world *world, uint32_t offset, int depth, t_vec min, t_vec max)
{
	int				i;
	t_accel_node	*node;
	t_vec			vec;

	i = 0;
	while (i < depth)
	{
		fprintf(stdout, "  ");
		i += 1;
	}
	node = &world->accel_nodes[offset];
	if (is_leaf(*node))
	{
#ifdef RT_VERBOSE
		if (nprims(*node) == 0)
			fprintf(stdout, "leaf %d (%f %f %f | %f %f %f), none (offset %d)\n", (int) nprims(*node), x(min), y(min), z(min), x(max), y(max), z(max), offset);
		else if (nprims(*node) == 1)
			fprintf(stdout, "leaf %d (%f %f %f | %f %f %f), one %d (offset %d)\n", (int) nprims(*node), x(min), y(min), z(min), x(max), y(max), z(max), node->a.one_primitive, offset);
		else
			fprintf(stdout, "leaf %d (%f %f %f | %f %f %f), first %d (offset %d)\n", (int) nprims(*node), x(min), y(min), z(min), x(max), y(max), z(max), world->accel_indices[node->a.primitive_ioffset], offset);
#else
		fprintf(stdout, "leaf %d\n", (int) nprims(*node));
#endif
	}
	else
	{
#ifdef RT_VERBOSE
		fprintf(stdout, "branch (%f %f %f | %f %f %f) (%f %d) (offset %d)\n", x(min), y(min), z(min), x(max), y(max), z(max), split_pos(*node), split_axis(*node), offset);
#else
		fprintf(stdout, "branch\n");
#endif
		vec = vec_set(max, split_axis(*node), split_pos(*node));
		dump_tree(world, offset + 1, depth + 1, min, vec);
		vec = vec_set(min, split_axis(*node), split_pos(*node));
		dump_tree(world, above_child(*node), depth + 1, vec, max);
	}
}

int
	main(int argc, char **argv)
{
	t_image			image;
	t_world			world;
	t_state			state;
	t_work			work;
	size_t			i;

	image.width = 1920;
	image.height = 1080;
	image.data = rt_malloc(sizeof(*image.data) * image.width * image.height);
	i = 0;
	while (i < image.width * image.height)
	{
		image.data[i].samples = 0;
		image.data[i].color = vec(0, 0, 0);
		i += 1;
	}
	state.image = &image;
	state.world = &world;
	world_create(&world);
	world.img_meta.width = image.width;
	world.img_meta.height = image.height;
	world.img_meta.samples = 100;
	if (argc == 1)
		world_gen(&world);
	else
		world_load(&world, argv[1]);
	world_accel(&world);
	printf("%d\n", (int) world.accel_nodes_count);
	/* dump_tree(&world, 0, 0, vec(-RT_HUGE_VAL, -RT_HUGE_VAL, -RT_HUGE_VAL), vec(RT_HUGE_VAL, RT_HUGE_VAL, RT_HUGE_VAL)); */
	work_create(&work, &state);
	work.work_size = world.img_meta.width * world.img_meta.height * world.img_meta.samples;
	work.work_index = 0;
	work.work_progress = 0;
	work_resume(&work);
	while (work.work_progress < work.work_size)
	{
		work_update(&work);
		usleep(10000);
	}
	rt_write_ppm("image.ppm", &image);
	work_destroy(&work);
	return (EXIT_SUCCESS);
}
