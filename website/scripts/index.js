var gnarwhal = gnarwhal || {};

gnarwhal.main = (main_event) => {
	let nav_items = document.querySelectorAll(".nav_item");
	let subpage_masks  = document.querySelectorAll(".subpage_mask" );

	function set_subpage(page_name) {
		gnarwhal.active_subpage.mask.style.width = "0";

		let mask = document.querySelector("#" + page_name + "_subpage_mask");
		mask.style.width = "100%";

		document.querySelector("#page_title").textContent = mask.dataset.name + " | Sploing One";

		gnarwhal.active_subpage = { name: page_name, mask: mask };
	}

	gnarwhal.active_subpage = { name: "", mask: subpage_masks[0] };
	window.history.pushState({ subpage: "" }, "", "/");

	for (let i = 0; i < nav_items.length; ++i) {
		nav_items[i].addEventListener("click", (click_event) => {
			let subpage = nav_items[i].dataset.subpage;

			if (subpage !== gnarwhal.active_subpage) {
				set_subpage(subpage);
				window.history.pushState({ subpage: subpage }, subpage, "/" + subpage);
			}
		});
	}

	window.addEventListener("popstate", (pop_event) => {
		set_subpage(pop_event.state.subpage);
	});
}

window.addEventListener("load", gnarwhal.main);
