<?php

/**
 * Export Drupal Commerce Purchase Order to Views.
 */

/**
 * Implements hook_views_data_alter()
 */
function commerce_purchase_order_views_data_alter(&$data) {
  $data['commerce_order']['po_number'] = array(
    'title' => t('PO Number'),
    'help' => t('The PO number associated with this order.'),
    'field' => array(
      'handler' => 'commerce_purchase_order_handler_field_po_number',
      'click sortable' => TRUE,
    ),
  );
}
