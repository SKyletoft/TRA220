{
	description = "A very basic flake";

	inputs = {
		nixpkgs.url     = "github:nixos/nixpkgs";
		flake-utils.url = "github:numtide/flake-utils";
	};

	outputs = { self, nixpkgs, flake-utils }:
		flake-utils.lib.eachDefaultSystem(system:
			let
				pkgs = import nixpkgs { inherit system; };
				custom-python = pkgs.python312.withPackages(p: with p; [
					matplotlib
					numpy
					scipy
					pyamg
					plotly
				]);
			in {
				devShells.default = pkgs.mkShell {
					nativeBuildInputs = with pkgs; [
						custom-python
						pyright
						ninja
						mold
						cmake
						clang-tools
						difftastic
						gnuplot
						futhark
						libglvnd
						SDL2
						glew
						bear
						gcc15
						hyperfine
					];
					shellHook = ''
						export HIPCC=/opt/rocm/hip/bin/hipcc
						export CC=/opt/rocm/hip/bin/hipcc
						export CXX=/opt/rocm/hip/bin/hipcc
						export CPATH=/opt/rocm/hip/include:$CPATH
					'';
				};
			}
		);
}
