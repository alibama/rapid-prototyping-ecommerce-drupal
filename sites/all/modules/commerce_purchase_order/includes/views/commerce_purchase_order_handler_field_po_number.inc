<?php
class commerce_purchase_order_handler_field_po_number extends views_handler_field {
  function construct() {
    parent::construct();

    //$this->additional_fields['uid'] = 'uid';
  }

  function render($values) {
    $order_id = $this->get_value($values, 'order_id');
    $text = '';
    $text = commerce_purchase_order_number_from_order_id($order_id);
    return $text;
  }
}
