# CUDA Class presentation

I want to present something other than performance numbers, I want to
discuss history and ergonomics.

# Graphics hacks

Originally graphics cards were very much fixed function hardware. They
were designed to do one kind of calculation and that was to produce
pretty images. The oldest graphics could do lines but more commonly
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
screen and embed other information in the colour? This is exactly what
they started doing and it's exactly what I've done here. I took the
first lab from the computer graphics course and replaced the output
rainbow with a single iteration of the poisson solver.

This is a massive pain. There is a bunch of boiler plate in passing
data back and forth. It's been made easier since 2004, but originally
all data you wanted to pass to the GPU had to either be small
constants or encoded as textures so you lost all your type
information. Texture reads are also often smoothed out unless you're
careful so you could easily mess up and lose precise information if
you set (or forgot to set) the correct texture sampler.

GLSL is also a dedicated language, so you can't really share code with
the host. HLSL is a thing on Windows and consoles but I'm entirely
unfamiliar with it beyond the fact that it exists.

But what benefits do you get? How much parallelisation do you get out
of it? The GPU will render the entire image in parallel, pixel by
pixel, generally tiled where tile sizes vary from 8x8 to 32x32, but
this is all hidden and doesn't matter for GPGPU when you render a
single rectangle with the same shader.

# HIP

Next I want to quickly talk about raw HIP, AMD's dialect of CUDA. I'm
sure many groups here have turned to raw CUDA. And as you either know
or are blissfully ignorant of, its a massive pain. We don't have to
think about mixed languages or compiling our shaders and sharing data
can now be done with raw memory you can treat in any way you like.

We do however get more control over how our programs are dispatched to
the GPU. We're limited by blocksizes, sure, but we don't just get to
set the output texture size. And we can even do 3D now. Technically we
can do 3D textures with the previous technique too, but we couldn't
back when it was the "normal" way of doing things, that came much
later.

But the provided type system is extremely flawed and too limited to be
of any use in catching incorrect programs. The main issue here is
still, as I ranted about at the half time presentations, the complete
lack of separation between host and device pointers.

# Futhark

So what's the future? This is still very much active research and
honestly, just leaving the hard parts to experts with CUDA is probably
more likely to just stay, but we can do better in theory.

I've been playing with a language called Futhark. It's a Danish
research project from DIKU, the compsci department at the University
of Copenhagen.

It's a purely functional ML dialect that targets the more native GPU
languages. Functional in the functional programming sense, meaning
competent and logically sound type systems, no side effects and no
mutation. A bit weird to get used to but much, much nicer than
imperative code once you get the hang of it.

Functional programs are usually built with higher order functions such
as map or reduce.  Map is the function that takes a function to
transform an element to another and an array to apply it to while
reduce takes an operation to merge two elements and an array to merge.
As you cannot have any side effects you can run all of this in an as
parallel a fashion as possible, and that it the goal of futhark,
letting the compiler find the parallelism for you and collapse nested
parallelism as flatly as possible.

I'm not showing any numbers as I realised this morning that I'd made a
major mistake in creating 2D arrays over and over again and I haven't
setup the language bindings to run it without a harness to read and
write to stdio so it's entirely incomparable, but the official
benchmark suite shows that generally outperforms naïvely written CUDA
but doesn't hold a candle to professionally massively optimised
kernels.
