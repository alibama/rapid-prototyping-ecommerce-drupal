<?php

function uva_prototype_lab_views_data() {
    return array(
        'commerce_line_item' => array(
            'volume_unit' => array(
                'field' => array(
                    'title' => t('Volume unit selection field'),
                    'help' => t('Adds a select list to change the volume units for the given line item.'),
                    'handler' => 'commerce_line_item_handler_field_edit_volume',
                )
            )
        ),
    );
}

class commerce_line_item_handler_field_edit_volume extends views_handler_field {

    function construct() {
        parent::construct();
        $this->additional_fields['line_item_id'] = 'line_item_id';
        $this->additional_fields['field_volume_unit'] = 'volume_unit';

        // Set real_field in order to make it generate a field_alias.
        $this->real_field = 'volume_unit';
    }

    function render($values) {
        return '<!--form-item-' . $this->options['id'] . '--' . $this->view->row_index . '-->';
    }

    /**
     * Returns the form which replaces the placeholder from render().
     */
    function views_form(&$form, &$form_state) {
        // The view is empty, abort.
        if (empty($this->view->result)) {
            return;
        }

        $form[$this->options['id']] = array(
            '#tree' => TRUE,
        );
        // At this point, the query has already been run, so we can access the results
        // in order to get the base key value (for example, nid for nodes).
        foreach ($this->view->result as $row_id => $row) {
            $line_item_id = $this->get_value($row, 'line_item_id');
            $unit = $this->get_value($row, 'volume_unit');

            $form[$this->options['id']][$row_id] = array(
                '#type' => 'select',
                '#default_value' => $unit,
                '#line_item_id' => $line_item_id,
                '#options' => list_allowed_values(field_info_field('field_volume_unit')),
            );
        }
    }

    function views_form_validate($form, &$form_state) {

    }

    function views_form_submit($form, &$form_state) {
        $field_name = $this->options['id'];
        foreach (element_children($form[$field_name]) as $row_id) {
            $line_item_id = $form[$field_name][$row_id]['#line_item_id'];
            // If the line item hasn't been deleted...
            if ($line_item = commerce_line_item_load($line_item_id)) {
                // And the quantity on the form is different...
                if ($form_state['values'][$field_name][$row_id] != $line_item->volume_unit) {
                    $line_item->volume_unit = $form_state['values'][$field_name][$row_id];

                    commerce_line_item_save($line_item);
                    entity_get_controller('commerce_line_item')->resetCache(array($line_item->line_item_id));
                }
            }
        }
    }
}
