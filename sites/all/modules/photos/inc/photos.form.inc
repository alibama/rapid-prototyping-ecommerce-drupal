<?php
/**
 * @file
 * Handles album settings form.
 */

/**
 * Node album settings form.
 */
function _photos_form($node) {
  global $user;
  if ($user->uid <> 1) {
    $t = photos_user_count();
    $msg = t('You may create %a albums, now you have created %b albums, you can still create %aa albums.', array('%a' => $t['total'], '%b' => $t['create'], '%aa' => $t['remain']));
    if (isset($t['rest']) && arg(2) <> 'edit') {
      drupal_set_message($msg);
      return drupal_goto('user/' . $user->uid);
    }
    $form['help'] = array('#value' => $msg, '#weight' => -10);
  }
  $type = node_type_get_type($node);
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($type->title_label),
    '#required' => TRUE,
    '#default_value' => isset($node->title) ? $node->title : NULL,
    '#weight' => -5
  );
  $photos_global = variable_get('photos_display_user', 1);
  $photos_page = variable_get('photos_display_page_user', 1);
  $photos_teaser = variable_get('photos_display_teaser_user', 1);
  $size = photos_upload_info(0);
  $image_styles = image_style_options(FALSE);
  $list_imagesize = variable_get('photos_display_list_imagesize', 'medium');
  $view_imagesize = variable_get('photos_display_view_imagesize', 'large');
  $full_imagesize = variable_get('photos_display_full_imagesize', 'large');
  $teaser_imagesize = variable_get('photos_display_teaser_imagesize', 'thumbnail');

  if (isset($node->album['list_imagesize'])) {
    $style_name = $node->album['list_imagesize'];
    // Necessary when upgrading from D6 to D7.
    if (isset($image_styles[$style_name])) {
      $list_imagesize = $style_name;
    }
  }

  if (isset($node->album['view_imagesize'])) {
    $style_name = $node->album['view_imagesize'];
    // Necessary when upgrading from D6 to D7.
    if (isset($image_styles[$style_name])) {
      $view_imagesize = $style_name;
    }
  }

  if (isset($node->album['full_imagesize'])) {
    $style_name = $node->album['full_imagesize'];
    // Necessary when upgrading from D6 to D7.
    if (isset($image_styles[$style_name])) {
      $full_imagesize = $style_name;
    }
  }

  if (isset($node->album['teaser_imagesize'])) {
    $style_name = $node->album['teaser_imagesize'];
    // Necessary when upgrading from D6 to D7.
    if (isset($image_styles[$style_name])) {
      $teaser_imagesize = $style_name;
    }
  }

  if ($photos_global || $photos_page || $photos_teaser) {
    $form['photos'] = array(
      '#type' => 'fieldset',
      '#title' => t('Album settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => -3
    );
    $opt = array(t('Do not display'), t('Display cover'), t('Display thumbnails'));
    if ($photos_global) {
      $form['photos']['global']['album'] = array(
        '#type' => 'fieldset',
        '#title' => t('Global Settings'),
        '#collapsible' => TRUE,
        '#tree' => TRUE
      );
      $form['photos']['global']['album']['viewpager'] = array(
        '#type' => 'select',
        '#default_value' => isset($node->album['viewpager']) ? $node->album['viewpager'] : variable_get('photos_display_viewpager', 10),
        '#title' => t('Images per page'),
        '#required' => TRUE,
        '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30, 35, 40, 45, 50))
      );
      $form['photos']['global']['album']['imageorder'] = array(
        '#type' => 'select',
        '#title' => t('Image display order'),
        '#required' => TRUE,
        '#default_value' => isset($node->album['imageorder']) ? $node->album['imageorder'] : variable_get('photos_display_imageorder', 'weight|asc'),
        '#options' => _photos_order_label()
      );

      $form['photos']['global']['album']['list_imagesize'] = array(
        '#type' => 'select',
        '#title' => t('Image size (list)'),
        '#required' => TRUE,
        '#default_value' => $list_imagesize,
        '#description' => t('Displayed in the list(e.g: photos/album/[nid]) of image size.'),
        '#options' => _photos_select_size()
      );
      $form['photos']['global']['album']['view_imagesize'] = array(
        '#type' => 'select',
        '#title' => t('Image size (page)'),
        '#required' => TRUE,
        '#default_value' => $view_imagesize,
        '#description' => t('Displayed in the page(e.g: photos/image/[fid]) of image size.'),
        '#options' => _photos_select_size()
      );
    }
    else {
      $form['photos']['global']['album'] = array(
        '#type' => 'value',
        '#value' => 'album',
        '#tree' => TRUE
      );
      $form['photos']['global']['album']['viewpager'] = array(
        '#type' => 'value',
        '#value' => isset($node->album['viewpager']) ? $node->album['viewpager'] : variable_get('photos_display_viewpager', 10),
      );
      $form['photos']['global']['album']['imageorder'] = array(
        '#type' => 'value',
        '#value' => isset($node->album['imageorder']) ? $node->album['imageorder'] : variable_get('photos_display_imageorder', 'weight|asc'),
      );
      $form['photos']['global']['album']['list_imagesize'] = array(
        '#type' => 'value',
        '#value' => $list_imagesize,
      );
      $form['photos']['global']['album']['view_imagesize'] = array(
        '#type' => 'value',
        '#value' => $view_imagesize,
      );
    }

    if ($photos_page) {
      $form['#attached']['js'][] = drupal_get_path('module', 'photos') . '/js/min/photos_form_full.min.js';
      $form['photos']['page']['album'] = array(
        '#type' => 'fieldset',
        '#title' => t('Page Settings'),
        '#collapsible' => TRUE,
        '#tree' => TRUE,
        '#prefix' => '<div id="photos-form-page">',
        '#suffix' => '</div>'
      );
      $form['photos']['page']['album']['page_display'] = array(
        '#type' => 'radios',
        '#default_value' => isset($node->album['page_display']) ? $node->album['page_display'] : variable_get('photos_display_page_display', 1),
        '#title' => t('Display setting'),
        '#required' => TRUE,
        '#options' => $opt
      );
      $form['photos']['page']['album']['full_viewnum'] = array(
        '#type' => 'select',
        '#default_value' => isset($node->album['full_viewnum']) ? $node->album['full_viewnum'] : variable_get('photos_display_full_viewnum', 10),
        '#title' => t('Quantity'),
        '#required' => TRUE,
        '#options' => drupal_map_assoc(array(5, 10, 15, 20, 25, 30, 35, 40, 45, 50)),
        '#prefix' => '<div class="photos-form-count">'
      );
      $form['photos']['page']['album']['full_imagesize'] = array(
        '#type' => 'select',
        '#title' => t('Image size'),
        '#required' => TRUE,
        '#default_value' => $full_imagesize,
        '#options' => _photos_select_size(),
        '#suffix' => '</div>'
      );
    }
    else {
      $form['photos']['page']['album'] = array(
        '#type' => 'value',
        '#value' => 'page',
        '#tree' => TRUE,
      );
      $form['photos']['page']['album']['page_display'] = array(
        '#type' => 'value',
        '#value' => isset($node->album['page_display']) ? $node->album['page_display'] : variable_get('photos_display_page_display', 1),
      );
      $form['photos']['page']['album']['full_viewnum'] = array(
        '#type' => 'value',
        '#value' => isset($node->album['full_viewnum']) ? $node->album['full_viewnum'] : variable_get('photos_display_full_viewnum', 10),
      );
      $form['photos']['page']['album']['full_imagesize'] = array(
        '#type' => 'value',
        '#value' => $full_imagesize,
      );
    }

    if ($photos_teaser) {
      $form['#attached']['js'][] = drupal_get_path('module', 'photos') . '/js/min/photos_form_teaser.min.js';
      $form['photos']['teaser']['album'] = array(
        '#type' => 'fieldset',
        '#title' => t('Teaser Settings'),
        '#collapsible' => TRUE,
        '#tree' => TRUE,
        '#prefix' => '<div id="photos-form-teaser">',
        '#suffix' => '</div>'
      );
      $form['photos']['teaser']['album']['teaser_display'] = array(
        '#type' => 'radios',
        '#default_value' => isset($node->album['teaser_display']) ? $node->album['teaser_display'] : variable_get('photos_display_teaser_display', 1),
        '#title' => t('Display setting'),
        '#required' => TRUE,
        '#options' => $opt
      );
      $form['photos']['teaser']['album']['teaser_viewnum'] = array(
        '#type' => 'select',
        '#default_value' => isset($node->album['teaser_viewnum']) ? $node->album['teaser_viewnum'] : variable_get('photos_display_teaser_viewnum', 5),
        '#title' => t('Quantity'),
        '#required' => TRUE,
        '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)),
        '#prefix' => '<div class="photos-form-count">'
      );
      $form['photos']['teaser']['album']['teaser_imagesize'] = array(
        '#type' => 'select',
        '#title' => t('Image size'),
        '#required' => TRUE,
        '#default_value' => $teaser_imagesize,
        '#options' => _photos_select_size(),
        '#suffix' => '</div>'
      );
    }
    else {
      $form['photos']['teaser']['album'] = array(
        '#type' => 'value',
        '#value' => 'teaser',
        '#tree' => TRUE,
      );
      $form['photos']['teaser']['album']['teaser_display'] = array(
        '#type' => 'value',
        '#value' => isset($node->album['teaser_display']) ? $node->album['teaser_display'] : variable_get('photos_display_teaser_display', 1),
      );
      $form['photos']['teaser']['album']['teaser_viewnum'] = array(
        '#type' => 'value',
        '#value' => isset($node->album['teaser_viewnum']) ? $node->album['teaser_viewnum'] : variable_get('photos_display_teaser_viewnum', 5),
      );
      $form['photos']['teaser']['album']['teaser_imagesize'] = array(
        '#type' => 'value',
        '#value' => $teaser_imagesize,
      );
    }
  }
  else {
    $form['photos']['global']['album'] = array(
      '#type' => 'value',
      '#value' => 'global',
      '#tree' => TRUE
    );
    $form['photos']['global']['album']['viewpager'] = array(
      '#type' => 'value',
      '#value' => isset($node->album['viewpager']) ? $node->album['viewpager'] : variable_get('photos_display_viewpager', 10),
    );
    $form['photos']['global']['album']['imageorder'] = array(
      '#type' => 'value',
      '#value' => isset($node->album['imageorder']) ? $node->album['imageorder'] : variable_get('photos_display_imageorder', 'weight|asc'),
    );
    $form['photos']['global']['album']['list_imagesize'] = array(
      '#type' => 'value',
      '#value' => $list_imagesize,
    );
    $form['photos']['global']['album']['view_imagesize'] = array(
      '#type' => 'value',
      '#value' => $view_imagesize,
    );
    $form['photos']['page']['album'] = array(
      '#type' => 'value',
      '#value' => 'page',
      '#tree' => TRUE,
    );
    $form['photos']['page']['album']['page_display'] = array(
      '#type' => 'value',
      '#value' => isset($node->album['page_display']) ? $node->album['page_display'] : variable_get('photos_display_page_display', 1),
    );
    $form['photos']['page']['album']['full_viewnum'] = array(
      '#type' => 'value',
      '#value' => isset($node->album['full_viewnum']) ? $node->album['full_viewnum'] : variable_get('photos_display_full_viewnum', 10),
    );
    $form['photos']['page']['album']['full_imagesize'] = array(
      '#type' => 'value',
      '#value' => $full_imagesize,
    );
    $form['photos']['teaser']['album'] = array(
      '#type' => 'value',
      '#value' => 'teaser',
      '#tree' => TRUE,
    );
    $form['photos']['teaser']['album']['teaser_display'] = array(
      '#type' => 'value',
      '#value' => isset($node->album['teaser_display']) ? $node->album['teaser_display'] : variable_get('photos_display_teaser_display', 1),
    );
    $form['photos']['teaser']['album']['teaser_viewnum'] = array(
      '#type' => 'value',
      '#value' => isset($node->album['teaser_viewnum']) ? $node->album['teaser_viewnum'] : variable_get('photos_display_teaser_viewnum', 5),
    );
    $form['photos']['teaser']['album']['teaser_imagesize'] = array(
      '#type' => 'value',
      '#value' => $teaser_imagesize,
    );
  }

  $pid = isset($node->album['pid']) ? $node->album['pid'] : NULL;
  $form['photos']['page']['album']['pid'] = array(
    '#type' => 'value',
    '#value' => $pid
  );

  return $form;
}
