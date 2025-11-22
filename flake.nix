{
	description = "A very basic flake";

	inputs = {
		nixpkgs.url     = "github:nixos/nixpkgs";
		flake-utils.url = "github:numtide/flake-utils";
	};

	outputs = { self, nixpkgs, flake-utils }:
		flake-utils.lib.eachDefaultSystem(system:
			let
				pkgs = nixpkgs.legacyPackages.${system};
				custom-python = pkgs.python312.withPackages(p: with p; [
					matplotlib
					numpy
				]);
			in {
				devShells.default = pkgs.mkShell {
					nativeBuildInputs = with pkgs; [
						custom-python
						pyright
						ninja
						cmake
						clang-tools
					];
					shellHook = ''
						export HIPCC=/opt/rocm/hip/bin/hipcc
						export CC=/opt/rocm/hip/bin/hipcc
						export CXX=/opt/rocm/hip/bin/hipcc
					'';
				};
			}
		);
}
