<?php

/**
 * @file
 * Definition of photos_handler_field_photos_album_cover.
 */

/**
 * Field handler to provide simple renderer that allows using a themed image.
 *
 * @ingroup views_field_handlers
 */
class photos_handler_field_photos_album_cover extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['pid'] = 'pid';
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_cover_to_album'] = array('default' => TRUE, 'bool' => TRUE);
    $options['image_style'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_cover_to_album'] = array(
      '#title' => t("Link to album page"),
      '#description' => t("Link the cover to the album page"),
      '#type' => 'checkbox',
      '#default_value' => $this->options['link_cover_to_album'],
    );

    $styles = image_styles();
    $style_options = array('' => t('Default'));
    foreach ($styles as $style) {
      $style_options[$style['name']] = $style['name'];
    }

    $form['image_style'] = array(
      '#title' => t('Image style'),
      '#description' => t('Using <em>Default</em> will display the text "View album".'),
      '#type' => 'select',
      '#options' => $style_options,
      '#default_value' => $this->options['image_style'],
    );
  }

  function render($values) {
    $pid = $this->get_value($values, 'pid');
    $fid = $this->get_value($values);
    if (empty($fid)) {
      $fid = db_query("SELECT fid FROM {photos_image} WHERE pid = :pid", array(':pid' => $pid))->fetchField();
    }
    if ($this->options['image_style'] && $fid) {
      $picture = file_load($fid);
      $picture_filepath = $picture->uri;
      if (file_valid_uri($picture_filepath)) {
        $output = theme('image_style', array('style_name' => $this->options['image_style'], 'path' => $picture_filepath));
        if ($this->options['link_cover_to_album'] && user_access('view photo')) {
          $output = l($output, "photos/album/$pid", array('html' => TRUE));
        }
      }
      else {
        $output = '';
      }
    }
    else {
      // @todo look up first image in album.
      $fid = db_query("SELECT fid FROM {photos_image} WHERE pid = :pid", array(':pid' => $pid));
      if ($this->options['link_cover_to_album']) {
        $output = l(t('View album'), "photos/album/$pid", array('html' => TRUE));
      }
      else {
        // @todo default output if no imagestyle selected.
        $output = '';
      }
    }

    return $output;
  }
}
