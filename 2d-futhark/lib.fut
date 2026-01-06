def iteration [n] [m] (pd: [n][m]f32) (b: [n][m]f32) (dx2: f32) (dy2: f32): [n][m]f32 =
  let per_square (x: i64) (y: i64) =
    if x == 0 || x >= n - 1 || y == 0 || y >= m - 1
    then pd[x, y]
    else ((pd[x, y+1] + pd[x, y - 1]) * dy2 +
          (pd[x+1, y] + pd[x-1, y]) * dx2 -
          b[x, y] * dx2 * dy2) / 2 * (dx2 + dy2)
  in iota n |> map \x -> iota m |> map \y -> per_square x y

def calc (nx: i64) (ny: i64) (nt: i64) (x_min: f32) (x_max: f32) (y_min: f32) (y_max: f32): [][]f32 =
  let p = replicate nx (replicate ny 0)
  let b = copy p with [ny/4, nx/4] = 100 with [3*ny/4, 3*nx/4] = -100
  let dx2 = (x_max - x_min) / f32.i64 (nx - 1)
  let dy2 = (y_max - y_min) / f32.i64 (ny - 1)
  in loop acc = copy p for _i < nt do
      iteration acc b dx2 dy2

entry main =
  calc 50 50 100 0.0 2.0 0.0 1.0
