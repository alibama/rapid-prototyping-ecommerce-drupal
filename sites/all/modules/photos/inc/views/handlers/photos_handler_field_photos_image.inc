<?php

/**
 * @file
 * Definition of photos_handler_field_photos_image.
 */

/**
 * Field handler to provide simple renderer that allows using a themed image.
 *
 * @ingroup views_field_handlers
 */
class photos_handler_field_photos_image extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();
    $options['link_photo_to_album_image'] = array('default' => TRUE, 'bool' => TRUE);
    $options['image_style'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_photo_to_album_image'] = array(
      '#title' => t("Link to image page"),
      '#description' => t("Link the image to the album image page"),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_photo_to_album_image'],
    );

    $styles = image_styles();
    $style_options = array('' => t('Default'));
    foreach ($styles as $style) {
      $style_options[$style['name']] = $style['name'];
    }

    $form['image_style'] = array(
      '#title' => t('Image style'),
      '#description' => t('Using <em>Default</em> will display the image fid.'),
      '#type' => 'select',
      '#options' => $style_options,
      '#default_value' => $this->options['image_style'],
    );
  }

  function render($values) {
    if ($this->options['image_style'] && module_exists('image')) {
      if ($picture_fid = $this->get_value($values)) {
        $picture = file_load($picture_fid);
        $picture_filepath = $picture->uri;
      }
      if (file_valid_uri($picture_filepath)) {
        $output = theme('image_style', array('style_name' => $this->options['image_style'], 'path' => $picture_filepath));
        if ($this->options['link_photo_to_album_image'] && user_access('view photo')) {
          $output = l($output, "photos/image/$picture_fid", array('html' => TRUE));
        }
      }
      else {
        $output = '';
      }
    }
    else {
      // @todo default output if no imagestyle selected.
      $output = $this->get_value($values);
    }

    return $output;
  }
}
