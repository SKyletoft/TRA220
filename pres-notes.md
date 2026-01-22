# CUDA Class presentation

I want to present something other than performance numbers, I want to
discuss history and ergonomics.

Originally graphics cards were very much fixed function hardware. They
were designed to do one kind of calculation and that was to produce
pretty images.  The oldest graphics could do lines but more commonly
had hardware accelerated text rendering, so you could draw 80x25
characters to a screen. Then we got tiled backgrounds and sprites, but
current graphics started in the 90s with fixed function 3D pipelines.

We could send 3D models and textures to the GPU and setup a could of
values for fixed lighting models and that worked fine enough, but as
graphics became more advanced we needed to run custom code on it, so
in 2002 shaders were introduced. This allowed us to generate colours
on the meshes per pixel per frame and with that we unlocked the first
GPGPU hack.

What if we just render a single rectangle that covers the entire
screen and embed other information in the colour?  This is exactly
what they started doing and it's exactly what I've done here.  I took
the first lab from the computer graphics course and replaced the
output rainbow with a single iteration of the poisson solver.

This is a massive pain.
