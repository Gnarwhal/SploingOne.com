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


	let gnarwhal_alternates = document.querySelectorAll(".gnarwhal_alternate");
	let gnarwhal_downloads  = document.querySelectorAll(".gnarwhal_download" );
	let gnarwhal_buttons    = document.querySelectorAll(".gnarwhal_download_button");

	for (let i = 0; i < gnarwhal_alternates.length; ++i) {
		gnarwhal_alternates[i].addEventListener("click", (click_event) => {
			if (gnarwhal_downloads[i].classList.contains("expanded")) {
				gnarwhal_downloads[i].classList.remove("expanded");
			} else {
				gnarwhal_downloads[i].classList.add("expanded");
			}
		});
	}

	for (let button of gnarwhal_buttons) {
		button.addEventListener("click", (click_event) => { click_event.stopPropagation(); });
	}
}

window.addEventListener("load", gnarwhal.main);
