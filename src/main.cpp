#include <hpx/hpx_init.hpp>
#include <hpx/include/components.hpp>

class tree: public hpx::components::component_base<tree> {

	std::vector<hpx::id_type> children;

public:
	tree(int depth, int next_proc, int branch_factor) {
		static const auto locs = hpx::find_all_localities();
		if (depth > 0) {
			std::vector<hpx::future<hpx::id_type>> futs(branch_factor);
			for (int ci = 0; ci < branch_factor; ci++) {
				const auto loc = locs[next_proc++ % locs.size()];
				futs[ci] = hpx::async([loc,depth,next_proc, branch_factor]() {
					return hpx::new_<tree>(loc, depth - 1, next_proc,
							branch_factor).get();
				});
			}
			for (int ci = 0; ci < branch_factor; ci++) {
				children.push_back(futs[ci].get());
			}
		}
	}
	;
};

HPX_REGISTER_COMPONENT(hpx::components::component<tree>, tree);

int hpx_main(int argc, char *argv[]) {
	int d;
	if( argc > 1 ) {
		d = atoi(argv[1]);
	} else {
		d = 3;
	}
	constexpr int bf = 8;
	{
		printf("Creating %i components\n", int(pow(bf, d)));
		hpx::new_<tree>(hpx::find_here(), d, 1, bf).get();
		printf("Done\n");
	}
	return hpx::finalize();

}

int main(int argc, char *argv[]) {
	std::vector<std::string> cfg = { "hpx.commandline.allow_unknown=1" };
	hpx::init(argc, argv, cfg);
}
