#ifndef RT_H
# define RT_H

# ifndef RT_MT
#  define RT_THREADS 1
#  define RT_ONESHOT 1
# else
#  define RT_ONESHOT 0
#  ifndef RT_THREADS
#   define RT_THREADS 8
#  endif
# endif

# ifndef RT_RENDER_CHUNK_SIZE
#  define RT_RENDER_CHUNK_SIZE 1
# endif

# include "rtmath.h"
# include "mt.h"
# include "gfx.h"
# include "scene.h"
# include "util.h"

typedef struct s_rt_state	t_rt_state;

struct s_rt_state {
	t_win		win;
	t_img		img;
	t_scene		scene;
	t_mutex		mtx;
	t_cond		cnd;
	t_thread	threads[RT_THREADS];
	size_t		idx;
	size_t		size;
	size_t		*order;
	int			running;
	long		version;
};

t_vec	trace_pixel(t_rt_state *state, int x, int y);

void	thread_start(t_rt_state *state);
void	thread_stop(t_rt_state *state);

void	render_range(t_rt_state *state, t_vec *dst, size_t begin, size_t end);
void	render_draw(t_rt_state *state, t_vec *dst, size_t begin, size_t end);

int	rt_exit(void *ctx);

#endif
