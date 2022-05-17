#ifndef SCENE_H
# define SCENE_H

# include "util.h"
# include "rtmath.h"
# include <stddef.h>

typedef struct s_entity_vt			t_entity_vt;
typedef struct s_entity				t_entity;
typedef struct s_ambient_lighting	t_ambient_lighting;
typedef struct s_camera				t_camera;
typedef struct s_light				t_light;
typedef struct s_sphere				t_sphere;
typedef struct s_plane				t_plane;
typedef struct s_cylinder			t_cylinder;

typedef struct s_hit				t_hit;
typedef struct s_scene				t_scene;

struct s_hit {
	t_vec	pos;
	t_vec	normal;
	double	t;
};

struct s_entity_vt {
	int	(*hit)(t_entity *ent, t_ray ray, t_hit *hit);
};

struct s_entity {
	const t_entity_vt	*vt;
};

struct s_ambient_lighting {
	t_entity	base;
	double		ratio;
	t_vec		color;
};

struct s_camera {
	t_entity	base;
	t_vec		pos;
	t_vec		dir;
	double		fov;	
};

struct s_light {
	t_entity	base;
	t_vec		pos;
	double		brightness;
	t_vec		color;
};

struct s_sphere {
	t_entity	base;
	t_vec		pos;
	double		diameter;
};

struct s_plane {
	t_entity	base;
	t_vec		pos;
	t_vec		dir;
};

struct s_cylinder {
	t_entity	base;
	t_vec		pos;
	t_vec		dir;
	double		diameter;
	double		height;
};

struct s_scene {
	t_entity			**entities;
	size_t				count;
	t_camera			*camera;
	t_ambient_lighting	*ambient_lighting;
};

const t_entity_vt	*sphere_vt(void);
int					sphere_hit(t_entity *ent, t_ray ray, t_hit *hit);

#endif
