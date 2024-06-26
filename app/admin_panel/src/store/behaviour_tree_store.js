import { writable } from 'svelte/store';

export const behaviour_tree_xml_code = writable("");

export const show_behaviour_tree_save_modal = writable(false);
export const behaviour_tree_save_modal_code = writable("");

export let behaviour_tree_list = writable([]);

async function main() {
    behaviour_tree_list.set(await api.getBehaviourTreeList());
}

main();