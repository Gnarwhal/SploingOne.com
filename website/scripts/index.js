var gnarwhal = gnarwhal || {};

gnarwhal.main = (main_event) => {
	let nav_items = document.querySelectorAll(".nav_item");
	let subpage_masks  = document.querySelectorAll(".subpage_mask" );

	gnarwhal.active_subpage = "";
	window.history.pushState({ subpage: "" }, "", "/");

	function set_subpage(page_name) {	
		for (let j = 0; j < subpage_masks.length; ++j) {
			subpage_masks[j].style.width = "0%";
		}

		document.querySelector("#" + page_name + "_subpage_mask").style.width = "100%";
	}

	for (let i = 0; i < nav_items.length; ++i) {
		nav_items[i].addEventListener("click", (click_event) => {
			let subpage = nav_items[i].dataset.subpage;

			if (subpage !== gnarwhal.active_subpage) {
				gnarwhal.active_subpage = subpage;
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
