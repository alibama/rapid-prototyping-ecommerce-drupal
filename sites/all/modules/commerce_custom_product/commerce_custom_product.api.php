<?php

/**
 * @file
 * Documents hooks provided by the Customizable Products module.
 */


/**
 * Allows modules to react to the creation of a new product line item type.
 *
 * @param $line_item_type
 *   The product line item type info array.
 * @param $skip_rebuild
 *   Boolean indicating whether or not this save will result in the menu being
 *   rebuilt; if FALSE, your hook should not perform similar rebuild operations.
 *
 * @see commerce_custom_product_line_item_type_save()
 */
function hook_commerce_custom_product_line_item_type_insert($line_item_type, $skip_rebuild = FALSE) {
  // No example.
}

/**
 * Allows modules to react to the update of a product line item type.
 *
 * @param $line_item_type
 *   The product line item type info array.
 * @param $skip_rebuild
 *   Boolean indicating whether or not this save will result in the menu being
 *   rebuilt; if FALSE, your hook should not perform similar rebuild operations.
 *
 * @see commerce_custom_product_line_item_type_save()
 */
function hook_commerce_custom_product_line_item_type_update($line_item_type, $skip_rebuild = FALSE) {
  // No example.
}

/**
 * Allows modules to react to the deletion of a product line item type.
 *
 * @param $line_item_type
 *   The product line item type info array.
 *
 * @see commerce_custom_product_line_item_type_delete()
 */
function hook_commerce_custom_product_line_item_type_delete($line_item_type) {
  // No example
}
