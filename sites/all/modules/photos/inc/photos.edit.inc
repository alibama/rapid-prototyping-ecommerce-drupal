<?php
/**
 * @file
 * Handles uploading and editing images.
 */

/**
 * Photos upload and edit page.
 */
function photos_edit_upload() {
  global $user;
  $v = '';
  $header = array(
    array('data' => t('Album title'), 'field' => 'n.title'),
    array('data' => t('Image count'), 'field' => 'p.count'),
    array('data' => t('Link')),
    array('data' => t('Create time'), 'field' => 'n.nid', 'sort' => 'desc'),
  );
  $query = db_select('node', 'n')->extend('TableSort')->extend('PagerDefault');
  $query->join('photos_album', 'p', 'p.pid = n.nid');
  $query->fields('n')
    ->fields('p', array('count'));
  $query->condition('n.uid', $user->uid);
  $query->orderByHeader($header);
  $query->limit(20);
  $result = $query->execute();

  foreach ($result as $node) {
    $slideshow = '';
    if (module_exists('dfgallery')) {
      // @todo add check and support for other slideshow options.
      $slideshow = l(t('Slideshow'), 'photos/Slide/' . $node->nid);
    }
    $table[] = array(
      l($node->title, 'node/' . $node->nid),
      $node->count,
      array(
        'data' => l(t('Upload'), 'node/' . $node->nid . '/photos') . l(t('View'), 'photos/album/' . $node->nid) . $slideshow,
        'class' => 'photos_album_edit_table_link'
      ),
      format_date($node->created, 'small')
    );
  }
  $edit = array('submit' => TRUE);
  $upload_form = drupal_get_form('photos_upload_form', 0, $edit);
  $v = drupal_render($upload_form);
  if (empty($table)) {
    $table[] = array(array('data' => l(t('Please create an album.'), 'node/add/photos'), 'colspan' => 4));
  }
  else {
    $v .= t('Or select an album') . ': ';
  }
  $v .= theme('table', array('header' => $header, 'rows' => $table));
  $v .= theme('pager');

  return $v;
}

/**
 * Upload form.
 */
function photos_upload_form($form, &$form_state, $node = FALSE, $edit = array()) {
  global $user;
  $form['new'] = array(
    '#title' => t('Image upload'),
    '#weight' => -4,
    '#type' => 'fieldset',
    '#collapsible' => TRUE
  );
  $allow_zip = ((variable_get('photos_upzip', 0)) ? ' zip' : '');
  if (isset($node->type) && $node->type == 'photos' || photos_get_count('user_album', $user->uid)) {
    // Check if plubload is installed.
    if (variable_get('photos_plupload_status', 0)) {
      $form['new']['plupload'] = array(
        '#type' => 'plupload',
        '#title' => t('Upload photos'),
        '#description' => t('Upload multiple images.'),
        '#autoupload' => TRUE,
        '#submit_element' => '#edit-submit',
        '#upload_validators' => array(
          'file_validate_extensions' => array('jpg jpeg gif png' . $allow_zip),
        ),
        '#plupload_settings' => array(
          'chunk_size' => '1mb',
        ),
      );
    }
    else {
      $form['new']['#description'] = t('Allow the type:') . ' jpg gif png jpeg' . $allow_zip;

      for ($i = 0; $i < variable_get('photos_num', 5); ++$i) {
        $form['new']['images_' . $i] = array(
          '#type' => 'file'
        );
        $form['new']['title_' . $i] = array(
          '#type' => 'textfield',
          '#title' => t('Image title'),
        );
        $form['new']['des_' . $i] = array(
          '#type' => 'textarea',
          '#title' => t('Image description'),
          '#cols' => 40,
          '#rows' => 3,
        );
      }
    }
  }
  else {
    drupal_set_message(t('You must first !create an album to upload images.', array(
      '!create' => l(t('create'), 'node/add/photos', array(
        'query' => drupal_get_destination()
      ))
    )));
    return $form;
  }
  if (isset($node->type) && $node->type == 'photos') {
    $form['new']['pid'] = array(
      '#type' => 'value',
      '#value' => $node->nid
    );
  }
  else {
    $form['#attached']['js'][] = drupal_get_path('module', 'photos') . '/js/min/photos_editlist.min.js';
    $form['new']['pid'] = array(
      '#title' => t('Upload to album'),
      '#type' => 'select',
      '#options' => _photos_useralbum_option($user->uid),
      '#default_value' => isset($_GET['pid']) ? $_GET['pid'] : NULL,
      '#required' => TRUE,
      '#prefix' => '<div id="photos-pid">',
      '#suffix' => '</div>',
      '#weight' => -5,
    );
  }
  if (isset($node->nid)) {
    $form['new']['nid'] = array(
      '#type' => 'value',
      '#value' => $node->nid
    );
  }
  if ($edit['submit']) {
    $form['new']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Confirm upload'),
      '#weight' => 10,
      '#submit' => array('photos_upload_form_submit'),
    );
    $form['#action'] = url($_GET['q'], array('query' => drupal_get_destination()));
    $form['#attributes']['enctype'] = 'multipart/form-data';
  }

  return $form;
}

/**
 * Submit upload form.
 */
function photos_upload_form_submit($form, &$form_state) {
  global $user;
  // $limits = _upload_file_limits($user); // @todo set limits for maximum upload size etc.
  $validators = array(
    'file_validate_is_image' => array(),
    // 'file_validate_image_resolution' => array($limits['resolution']),
    // 'file_validate_size' => array($limits['file_size'], $limits['user_size'])
  );
  $files_uploaded = array();
  if (variable_get('photos_plupload_status', 0)) {
    $nid = isset($form_state['values']['nid']) ? $form_state['values']['nid'] : $form_state['values']['pid'];
    $album_uid = db_query("SELECT uid FROM {node} WHERE nid = :nid", array(':nid' => $nid))->fetchField();
    $account = user_load($album_uid);
    foreach ($form_state['values']['plupload'] as $uploaded_file) {
      if ($uploaded_file['status'] == 'done') {
        // Check for zip files.
        $ext = drupal_substr($uploaded_file['name'], -3);
        if ($ext <> 'zip' && $ext <> 'ZIP') {
          $file = new stdClass();
          $photos_path = photos_check_path('default', '', $account);
          $photos_name = _photos_rename($uploaded_file['name']);
          $file->uri = file_destination($photos_path . '/' . $photos_name, FILE_EXISTS_RENAME);
          if (file_unmanaged_move($uploaded_file['tmppath'], $file->uri)) {
            $info = image_get_info($file->uri);
            if ($info['extension'] && $info['width']) {
            // @todo add limits?
            // $limits = _upload_file_limits($ac);
            /* $validators = array(
              // 'file_validate_image_resolution' => array($limits['resolution']),
              // '_file_validate_size' => array($limits['file_size'], $limits['user_size'], $ac)
            ); */
              $file->pid = $form_state['values']['pid'];
              $file->nid = isset($form_state['values']['nid']) ? $form_state['values']['nid'] : $form_state['values']['pid'];
              $file->uid = $account->uid;
              $file->filename = $photos_name;
              $file->filesize = $info['file_size'];
              $file->filemime = $info['mime_type'];
              if ($file->fid = _photos_save_data($file)) {
                $files_uploaded[] = photos_image_date($file);
              }
            }
            else {
              file_delete($file->uri);
              watchdog('photos_swfu', 'Wrong file type');
            }        
          }
          else {
            watchdog('photos_swfu', 'Upload error. Could not move temp file.');
          }
        }
        else {
          if (!variable_get('photos_upzip', 0)) {
            drupal_set_message(t('Please set Album photos to open zip uploads.'), 'error');
          }
          $directory = photos_check_path();
          file_prepare_directory($directory);
          $zip = file_destination($directory . '/' . $uploaded_file['name'], FILE_EXISTS_RENAME);
          if (file_unmanaged_move($uploaded_file['tmppath'], $zip)) {
            $value = new stdClass();
            $value->pid = $form_state['values']['pid'];
            $value->nid = isset($form_state['values']['nid']) ? $form_state['values']['nid'] : $form_state['values']['pid'];
            $value->title = $uploaded_file['name'];
            $value->des = '';
            if (!$msg = _photos_unzip($zip, $value)) {
              drupal_set_message(t('Zip upload failed.'), 'error');
            }
          }
        }
      }
      else {
        drupal_set_message(t('Error uploading some photos.'), 'error');
      }
    }
  }
  else {
    $photos_num = variable_get('photos_num', 5);
    for ($i = 0; $i < $photos_num; ++$i) {
      _photos_rename();
      if ($_FILES['files']['name']['images_' . $i]) {
        $value = new stdClass();
        $value->pid = $form_state['values']['pid'];
        $value->nid = isset($form_state['values']['nid']) ? $form_state['values']['nid'] : $form_state['values']['pid'];
        $value->des = $form_state['values']['des_' . $i];
        $value->title = $form_state['values']['title_' . $i];
        $ext = drupal_substr($_FILES['files']['name']['images_' . $i], -3);
        if ($ext <> 'zip' && $ext <> 'ZIP') {
          if ($file = file_save_upload('images_' . $i, $validators, photos_check_path())) {
            $files_uploaded[] = photos_image_date((object)array_merge((array)$file, (array)$value), 1);
          }
        }
        else {
          if (!variable_get('photos_upzip', 0)) {
            return form_set_error('error', t('Please set Album photos to open zip upload'));
          }
          $directory = photos_check_path();
          file_prepare_directory($directory);
          $zip = file_destination($directory . '/' . trim(basename($_FILES['files']['name']['images_' . $i])), FILE_EXISTS_RENAME);
          if (move_uploaded_file($_FILES['files']['tmp_name']['images_' . $i], $zip)) {
            if (!$msg = _photos_unzip($zip, $value)) {
              $msg = t('Failed to upload');
            }
          }
        }
      }
    }
  }
  if (!empty($files_uploaded)) {
    drupal_set_message(t('%count image(s) uploaded successfully.', array('%count' => count($files_uploaded))));
  }
  if (isset($msg)) {
    drupal_set_message($msg);
  }
}

/**
 * Image management.
 */
function photos_edit_page($node) {
  $output = '';
  if (isset($_POST['ajax'])) {
    // Reload photos editlist form after uploading new photos.
    $nid = $node->nid;
    $images = _photos_edit_page_album_images($nid);
    $output .= _photos_prep_editlist($images);
    print $output;
    drupal_exit();
  }
  if ($node->type == 'photos') {
    $edit_form = drupal_get_form('photos_upload_form', $node, array('submit' => TRUE));
    $output = '<div id="photos_upload_new">' . drupal_render($edit_form) . '</div>';
    $output .= _photos_edit_page_album($node);
    $page_title = t('Organize Albums: @title', array('@title' => $node->title));
    drupal_set_title($page_title, PASS_THROUGH);
  }
  else {
    $edit_form = drupal_get_form('photos_upload_form', $node, array('submit' => TRUE));
    $output = '<div id="photos_upload_new">' . drupal_render($edit_form) . '</div>';
    $output .= _photos_edit_page_node($node);
    $page_title = t('Organize Sub-Album: @title', array('@title' => $node->title));
    drupal_set_title($page_title, PASS_THROUGH);
  }

  return $output;
}

/**
 * Grid drag and drop image sorting.
 */
function photos_edit_sort_page($node) {
  $type = 'album';
  if (isset($node->subalbum)) {
    $type = 'sub_album';
  }
  $output = '';
  $update_button = '';
  if (isset($node->album['imageorder']) && $node->album['imageorder'] <> 'weight|asc') {
    $update_button = ' ' . t('Update album image display order to "Weight - smallest first".');
  }
  $nid = $node->nid;
  drupal_add_library('system', 'ui');
  drupal_add_library('system', 'ui.draggable');
  drupal_add_library('system', 'ui.sortable');
  drupal_add_js(array('photos' => array('pid' => $nid)), 'setting');
  $images = array();
  if ($type == 'album') {
    drupal_add_js(array('photos' => array('sort' => 'images')), 'setting');
    $images = _photos_edit_page_album_images($nid, 50);
  }
  elseif ($type == 'sub_album') {
    drupal_add_js(array('photos' => array('sort' => 'sub_album')), 'setting');
    $images = _photos_edit_page_sub_album_images($nid, 50);
  }
  $count = count($images);
  // @todo check album default sort - if not weight|asc show button to update album.
  $output .= t('Limit') . ': ' . l(100, 'node/' . $nid . '/photos-sort', array('query' => array('limit' => 100)));
  $output .= ' - ' . l(500, 'node/' . $nid . '/photos-sort', array('query' => array('limit' => 500)));
  $default_message = t('%img_count images to re-arrange.', array('%img_count' => $count));
  $output .= '<div id="photos-sort-message">' . $default_message . $update_button . ' ' . '<span id="photos-sort-updates"></span></div>';
  $output .= '<ul id="photos-sortable">';
  foreach ($images as $image) {
    // $image->user = user_load($image->uid);
    // $image->href = 'photos/image/' . $image->fid;
    $item = array();
    $title = $image->title;
    // @todo set photos_sort_style variable for custom image style settings.
    $style_name = variable_get('photos_thumb_size', 'thumbnail');
    $output .= '<li id="photos_' . $image->fid . '" class="photos-sort-grid ui-state-default">';
    $output .= theme('image_style', array('style_name' => $style_name, 'path' => $image->uri, 'alt' => $title, 'title' => $title));
    $output .= '</li>';
  }
  $output .= '</ul>';
  $page_title = t('Re-arrange Photos: @title', array('@title' => $node->title));
  drupal_set_title($page_title, PASS_THROUGH);

  return $output;
}

/**
 * Grid drag and drop album sorting.
 */
function photos_edit_sort_album_page($account) {
  $output = '';
  $uid = $account->uid;
  drupal_add_library('system', 'ui');
  drupal_add_library('system', 'ui.draggable');
  drupal_add_library('system', 'ui.sortable');
  drupal_add_js(array('photos' => array('uid' => $uid)), 'setting');
  drupal_add_js(array('photos' => array('sort' => 'albums')), 'setting');
  $albums = _photos_edit_page_albums($uid);
  $count = count($albums);
//  $output .= t('Limit') . ': ' . l(100, 'node/' . $nid . '/photos-sort', array('query' => array('limit' => 100)));
//  $output .= ' - ' . l(500, 'node/' . $nid . '/photos-sort', array('query' => array('limit' => 500)));
  $default_message = t('%album_count albums to re-arrange.', array('%album_count' => $count));
  $output .= '<div id="photos-sort-message">' . $default_message . ' ' . '<span id="photos-sort-updates"></span></div>';
  $output .= '<ul id="photos-sortable">';
  // watchdog('photos', 'albums: <pre>' . print_r($albums, 1) . '</pre>');
  foreach ($albums as $album) {
    $item = array();
    $title = $album['title'];
    $cover = file_load($album['fid']);
    // @todo set photos_sort_style variable for custom image style settings.
    $style_name = variable_get('photos_thumb_size', 'thumbnail');
    $output .= '<li id="photos_' . $album['nid'] . '" class="photos-sort-grid ui-state-default">';
    $output .= theme('image_style', array('style_name' => $style_name, 'path' => $cover->uri, 'alt' => $title, 'title' => $title));
    $output .= '</li>';
  }
  $output .= '</ul>';
  //$page_title = t('Re-arrange Photos: @title', array('@title' => $node->title));
  //drupal_set_title($page_title, PASS_THROUGH);

  return $output;
}

/**
 * Save weight for array of image id's.
 */
function photos_edit_sort_save($order = array(), $nid = 0) {
  if ($nid) {
    $access = FALSE;
    if ($nid) {
      $node = node_load($nid);
      // Check for node_accss.
      $access = _photos_access('editAlbum', $node);
    }
    if ($access) {
      $weight = 0;
      // Update weight for all images in array / album.
      foreach ($order as $image_id) {
        $fid = str_replace('photos_', '', $image_id);
        db_query("UPDATE {photos_image} SET wid = :wid WHERE fid = :fid AND pid = :pid",
          array(':wid' => $weight, ':fid' => $fid, ':pid' => $nid));
        $weight++;
      }
      if ($weight > 0) {
        $message = t('Image order saved!');
        return $message;
      }
    }
  }
}

/**
 * Save weight for array of image id's in sub-album.
 */
function photos_edit_sort_subalbums_save($order = array(), $nid = 0) {
  if ($nid) {
    $access = FALSE;
    if ($nid) {
      $node = node_load($nid);
      // Check for node_accss.
      $access = _photos_access('editAlbum', $node);
    }
    if ($access) {
      $weight = 0;
      // Update weight for all images in array / album.
      foreach ($order as $image_id) {
        $fid = str_replace('photos_', '', $image_id);
        db_query("UPDATE {photos_node} SET wid = :wid WHERE fid = :fid AND nid = :nid",
          array(':wid' => $weight, ':fid' => $fid, ':nid' => $nid));
        $weight++;
      }
      if ($weight > 0) {
        $message = t('Image order saved!');
        return $message;
      }
    }
  }
}

/**
 * Save weight for array of album id's.
 */
function photos_edit_sort_albums_save($order = array(), $uid = 0) {
  if ($uid) {
    global $user;
    $access = FALSE;
    // @todo add support for admin.
    if ($user->uid == $uid) {
      $weight = 0;
      // Update weight for all albums in array.
      foreach ($order as $album_id) {
        $pid = str_replace('photos_', '', $album_id);
        $node = node_load($pid);
        // Check for node_accss.
        $access = _photos_access('editAlbum', $node);
        if ($access) {
          db_query("UPDATE {photos_album} SET wid = :wid WHERE pid = :pid",
            array(':wid' => $weight, ':pid' => $pid));
          $weight++;
        }
      }
      if ($weight > 0) {
        $message = t('Album order saved!');
        return $message;
      }
    }
  }
}

/**
 * Sub-album image management page.
 */
function _photos_edit_page_node($node) {
  $output = '';
  $result = array();
  if (isset($node->subalbum['count'])) {
    $column = isset($_GET['field']) ? $_GET['field'] : '';
    $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
    $term = _photos_order_value($column, $sort, 10, array('column' => 'a.wid', 'sort' => 'asc'));
    // Override weight sort for sub albums.
    if ($term['order']['column'] == 'p.wid') $term['order']['column'] = 'a.wid';
    $query = db_select('file_managed', 'f')
      ->extend('PagerDefault');
    $query->join('photos_node', 'a', 'a.fid = f.fid');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->join('node', 'n', 'n.nid = p.pid');
    $query->join('users', 'u', 'u.uid = f.uid');
    $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'));
    $query->fields('p', array('fid', 'pid', 'title', 'des', 'count', 'comcount', 'exif'));
    $query->fields('a', array('wid'));
    $query->fields('n', array('nid'));
    $query->addField('n', 'title', 'album_title');
    $query->fields('u', array('uid', 'name'));
    $query->condition('a.nid', $node->nid);
    $query->orderBy($term['order']['column'], $term['order']['sort']);
    $query->limit($term['limit']);
    $result = $query->execute();
    $output .= _photos_order_link('node/' . $node->nid . '/photos', $node->subalbum['count'], l(t('Album view'), 'photos/sub_album/' . $node->nid), 1);
    $output .= '<div class="messages warning">' . t('Move out: Move image out of the sub gallery, but it will not delete it.') . '</div>';
  }
  else {
    return;
  }

  $images = array();
  foreach ($result as $data) {
    $images[] = photos_get_info(0, $data);
  }
  if (isset($images[0]->fid)) {
    $images[0]->info = array(
      'title' => $node->title,
      'nid' => $node->nid,
      'uid' => $node->uid
    );
  }

  // Uploaded photos.
  $editlist_form = drupal_get_form('photos_editlist_form', $images, 'node');
  $output .= theme('pager');
  $output .= drupal_render($editlist_form);
  $output .= theme('pager');

  return $output;
}

/**
 * Get albums for album re-arrange page.
 */
function _photos_edit_page_albums($uid) {
  $albums = array();
  $query = db_select('node', 'n');
  $query->join('photos_album', 'p', 'p.pid = n.nid');
  $query->fields('n', array('nid', 'title'));
  $query->fields('p', array('wid', 'fid', 'count'));
  $query->condition('n.uid', $uid);
  $query->orderBy('p.wid', 'ASC');
  $query->orderBy('n.nid', 'DESC');
  $result = $query->execute();

  foreach ($result as $data) {
    if (isset($data->fid) && $data->fid <> 0) {
      $cover_fid = $data->fid;
    }
    else {
      $cover_fid = db_query("SELECT fid FROM {photos_image} WHERE pid = :pid", array(':pid' => $data->nid))->fetchField();
      if (empty($cover_fid)) {
        // Skip albums with no images.
        continue;
      }
    }
    $albums[] = array(
      'wid' => $data->wid,
      'nid' => $data->nid,
      'fid' => $cover_fid,
      'count' => $data->count,
      'title' => $data->title
    );
  }
  return $albums;
}

/**
 * Get images for sub album re-arrange page.
 */
function _photos_edit_page_sub_album_images($nid, $limit = 10) {
  $images = array();
  $column = isset($_GET['field']) ? $_GET['field'] : '';
  $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
  $term = _photos_order_value($column, $sort, $limit, array('column' => 's.wid', 'sort' => 'asc'));
  $query = db_select('file_managed', 'f')
    ->extend('PagerDefault');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('photos_node', 's', 's.fid = f.fid');
  $query->join('users', 'u', 'f.uid = u.uid');
  $query->join('node', 'n', 'n.nid = p.pid');
  $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'));
  $query->fields('p');
  $query->fields('u', array('uid', 'name'));
  $query->condition('s.nid', $nid);
  $query->limit($term['limit']);
  $query->orderBy($term['order']['column'], $term['order']['sort']);
  $query->addTag('node_access');
  $result = $query->execute();
  foreach ($result as $data) {
    $images[] = photos_get_info(0, $data);
  }
  if (isset($images[0]->fid)) {
    $node = node_load($nid);
    $images[0]->info = array(
      'pid' => $node->nid,
      'title' => $node->title,
      'uid' => $node->uid
    );
  }
  return $images;
}

/**
 * Get images for album management page.
 */
function _photos_edit_page_album_images($nid, $limit = 10) {
  $images = array();
  $column = isset($_GET['field']) ? $_GET['field'] : '';
  $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
  $term = _photos_order_value($column, $sort, $limit, array('column' => 'p.wid', 'sort' => 'asc'));
  $query = db_select('file_managed', 'f')
    ->extend('PagerDefault');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('users', 'u', 'f.uid = u.uid');
  $query->join('node', 'n', 'n.nid = p.pid');
  $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'));
  $query->fields('p');
  $query->fields('u', array('uid', 'name'));
  $query->condition('p.pid', $nid);
  $query->limit($term['limit']);
  $query->orderBy($term['order']['column'], $term['order']['sort']);
  $query->addTag('node_access');
  $result = $query->execute();
  foreach ($result as $data) {
    $images[] = photos_get_info(0, $data);
  }
  if (isset($images[0]->fid)) {
    $node = node_load($nid);
    $images[0]->info = array(
      'pid' => $node->nid,
      'title' => $node->title,
      'uid' => $node->uid
    );
    if (isset($node->album['cover'])) {
      $images[0]->info['cover'] = $node->album['cover'];
    }
  }
  return $images;
}

/**
 * Album image management page.
 */
function _photos_edit_page_album($node) {
  $output = '';
  if (isset($node->album['count'])) {
    $album_count = $node->album['count'];
  }
  else {
    $album_count = 0;
  }
  if (isset($node->album['count'])) {
    $images = _photos_edit_page_album_images($node->nid);
    $output .= _photos_order_link('node/' . $node->nid . '/photos', $node->album['count'], l(t('Album view'), 'photos/album/' . $node->nid), 1);
  }
  else {
    return;
  }
  $output .= '<div id="photos-editlist-form-wrapper">';
  $output .= _photos_prep_editlist($images);
  $output .= '</div>';
  return $output;
}

/**
 * Prepare photos editlist form.
 */
function _photos_prep_editlist($images = array()) {
  $output = '';
  $output .= theme('pager');
  $edit_form = drupal_get_form('photos_editlist_form', $images);
  $output .= drupal_render($edit_form);
  $output .= theme('pager');
  return $output;
}

/**
 * Album image management page.
 */
function _photos_edit_page_single_image($fid) {
  $output = '';
  $images = array();
  $query = db_select('file_managed', 'f');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('users', 'u', 'f.uid = u.uid');
  $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'));
  $query->fields('p');
  $query->fields('u', array('uid', 'name'));
  $query->condition('f.fid', $fid);
  $result = $query->execute();

  foreach ($result as $data) {
    $images[] = photos_get_info(0, $data);
  }

  if (isset($images[0]->fid)) {
    $node = node_load($images[0]->pid);
    $images[0]->info = array(
      'cover' => $node->album['cover'],
      'pid' => $node->nid,
      'title' => $node->title,
      'uid' => $node->uid
    );
    $edit_form = drupal_get_form('photos_editlist_form', $images);
    $output .= drupal_render($edit_form);
  }

  if (module_exists('colorbox')) {
    // Display in modal colorbox.
    print $output;
  }
  else {
    // Full page.
    return $output;
  }
}

/**
 * Edit list form.
 */
function photos_editlist_form($form, &$form_state, $images = array(), $type = 'album') {
  global $user;
  // Check for node id.
  if (empty($images)) {
    $nid = arg(1);
  }
  else {
    if (isset($images[0]->info['nid'])) {
      $nid = $images[0]->info['nid'];
    }
    else {
      $nid = $images[0]->info['pid'];
    }
  }
  if ($type == 'album') {
    $album_update = '';
    if (isset($images[0]) && $user->uid <> $images[0]->info['uid']) {
      $title = isset($images[0]->info['title']) ? $images[0]->info['title'] : '';
      $album_update = array($nid, $images[0]->info['title']);
    }
    else {
      $album_update = '';
    }
    $uid = isset($images[0]) ? $images[0]->uid : $user->uid;
    $album_pid = _photos_useralbum_option($uid, $album_update);
    $del_label = _photos_del_checkbox(0, t('Delete'));
    if (isset($images[0]->fid)) {
      $form['cover_fid'] = array('#type' => 'hidden', '#default_value' => $images[0]->fid);
    }
    $form['oldpid'] = array('#type' => 'hidden', '#default_value' => $nid);
    $submit = 'photos_editlist_submit';
  }
  else {
    $del_label = _photos_del_checkbox(0, t('Move out'));
    $submit = 'photos_editlist_submit_node';
    $form['nid'] = array('#type' => 'hidden', '#default_value' => $nid);
  }
  $form['photos']['#tree'] = TRUE;
  $thumb = variable_get('photos_title_0', FALSE);
  foreach ($images as $image) {
    $form['photos'][$image->fid]['del'] = $del_label;
    $image->user = user_load($image->uid);
    $image->href = 'photos/image/' . $image->fid;
    $item = array();
    $title = $image->title;
    $style_name = variable_get('photos_thumb_size', 'thumbnail');
    $image_view = theme('image_style', array(
      'style_name' => $style_name,
      'path' => $image->uri,
      'alt' => $title,
      'title' => $title
    ));
    $item[] = l($image_view, $image->href, array(
      'html' => TRUE,
      'attributes' => array('title' => $title)
    ));
    if ($type == 'album' && $images[0]->fid <> $image->fid) {
      $item[] = l(t('Set to Cover'), 'node/' . $image->pid . '/photos/cover/' . $image->fid);
    }
    if (isset($image->filesize)) {
      $item[] = t('Filesize: !size KB', array('!size' => round($image->filesize/1024)));
    }
    if (isset($image->count)) {
      $item[] = t('Visits: !count', array('!count' => $image->count));
    }
    if (isset($image->comcount)) {
      $item[] = t('Comments: !count', array('!count' => $image->comcount));
    }
    $form['photos'][$image->fid]['path']['#markup'] = theme('item_list', array('items' => $item));
    $form['photos'][$image->fid]['des'] = array(
      '#title' => t('Image description'),
      '#type' => 'textarea',
      '#default_value' => isset($image->des) ? $image->des : '',
      '#cols' => 40,
      '#rows' => 4
    );
    $form['photos'][$image->fid]['title'] = array(
      '#title' => t('Image title'),
      '#type' => 'textfield',
      '#default_value' => isset($image->title) ? $image->title : '',
      '#required' => FALSE
    );
    $form['photos'][$image->fid]['wid'] = array(
      '#title' => t('Weight'),
      '#type' => 'textfield',
      '#size' => 5,
      '#default_value' => isset($image->wid) ? $image->wid : NULL,
    );
    $form['photos'][$image->fid]['filepath'] = array('#type' => 'value', '#value' => $image->uri);
    if ($type == 'album') {
      $upload_info = t('Uploaded on !time by !name', array(
        '!name' => theme('username', array('account' => $image->user)),
        '!time' => format_date($image->timestamp, 'small')
      ));
      $form['photos'][$image->fid]['pid'] = array(
        '#title' => t('Move to the album'),
        '#type' => 'select',
        '#options' => $album_pid,
        '#default_value' => $image->pid,
        '#required' => TRUE
      );
    }
    else {
      $upload_info = t('Uploaded by !name on !time to !title', array(
        '!name' => theme('username', array('account' => $image->user)),
        '!time' => format_date($image->timestamp, 'small'),
        '!title' => l($image->album_title, 'node/' . $image->pid)
      ));
    }
    $form['photos'][$image->fid]['time']['#markup'] = $upload_info;
    $form['photos'][$image->fid]['uid'] = array('#type' => 'hidden', '#default_value' => $image->uid);
    $form['photos'][$image->fid]['oldtitle'] = array('#type' => 'hidden', '#default_value' => $image->title);
  };
  if (!empty($images)) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Confirm changes'),
      '#submit' => array($submit),
    );
  }

  return $form;
}

/**
 * Edit list form submit for sub-album.
 */
function photos_editlist_submit_node($form, &$form_state) {
  foreach ($form_state['values']['photos'] as $fid => $key) {
    if (!empty($key['del'])) {
      $msg[] = db_query('DELETE FROM {photos_node} WHERE fid = :fid AND nid = :nid',
        array(':fid' => $fid, ':nid' => $form_state['values']['nid']));
    }
    else {
      $update_fields = array(
        'des' => $key['des'],
      );
      if ($key['title'] <> $key['oldtitle']) {
        $update_fields['title'] = $key['title'];
      }
      db_merge('photos_image')
        ->key(array(
          'fid' => $fid
        ))
        ->fields($update_fields)
        ->execute();
      if ($key['wid']) {
        db_update('photos_node')
          ->fields(array(
            'wid' => $key['wid']
          ))
          ->condition('fid', $fid)
          ->condition('nid', $form_state['values']['nid'])
          ->execute();
      }
    }
  }
  if (isset($msg)) {
    photos_set_count('node_node', $form_state['values']['nid']);
    drupal_set_message(t('%count images are move out.', array('%count' => count($msg))));
  }
}

/**
 * Submit modifications to existing images.
 */
function photos_editlist_submit($form, &$form_state) {
  foreach ($form_state['values']['photos'] as $fid => $key) {
    if (!empty($key['del'])) {
      if ($form_state['values']['cover_fid'] == $fid) {
        db_update('photos_album')
          ->fields(array(
            'fid' => 0
          ))
          ->condition('pid', $form_state['values']['oldpid'])
          ->execute();
      }
      $msg[] = photos_file_del($fid, $key['filepath']);
      $uids[] = $key['uid'];
    }
    else {
      db_update('photos_image')
        ->fields(array(
          'pid' => $key['pid'],
          'des' => $key['des'],
          'wid' => $key['wid']
        ))
        ->condition('fid', $fid)
        ->execute();

      if ($key['title'] <> $key['oldtitle']) {
        db_update('photos_image')
          ->fields(array(
            'title' => check_plain($key['title'])
          ))
          ->condition('fid', $fid)
          ->execute();
      }
      if ($key['pid'] <> $form_state['values']['oldpid']) {
        $sub_select = db_select('photos_comment', 'v')
          ->fields('v', array('cid'))
          ->condition('v.fid', $fid)
          ->execute()->fetchCol();
        if (!empty($sub_select)) {
          db_update('comment')
            ->fields(array(
              'nid' => $key['pid']
            ))
            ->condition('cid', $sub_select, 'IN')
            ->execute();
        }
        $pid[] = $key['pid'];
        $uids[] = $key['uid'];
      }
    }
  }
  if (isset($msg)) {
    $pid[] = $form_state['values']['oldpid'];
    drupal_set_message(t('%count images are deleted.', array('%count' => count($msg))));
  }
  if (isset($pid) && count($pid)) {
    foreach ($pid as $nid) {
      _comment_update_node_statistics($nid);
      photos_set_count('node_album', $nid);
    }
    $uid = array_unique($uids);
    foreach ($uid as $id) {
      photos_set_count('user_image', $id);
    }
  }
}

/**
 * Handles image edit list delete checkbox.
 */
function _photos_del_checkbox($t = 0, $title = FALSE) {
  if ($t) {
    return (($t == 1) ? t('Delete') : $t) . '<input type="checkbox" name="del_check" id="del_check" />';
  }
  else {
    return array('#title' => $title . ' ', '#type' => 'checkbox', '#prefix' => '<div class="edit-del-all">', '#suffix' => '</div>');
  }
}

/**
 * Theme photos edit list.
 */
function theme_photos_editlist_form($variables) {
  $form = $variables['form'];
  $header = array(
    array('data' => _photos_del_checkbox(t('Select All'))),
    array('data' => t('Thumbnails')),
    array('data' => t('Content')),
  );
  $rows = array();
  foreach (element_children($form['photos']) as $key) {
    $rows[] = array(
      drupal_render($form['photos'][$key]['wid']) . drupal_render($form['photos'][$key]['del']),
      array(
        'data' => drupal_render($form['photos'][$key]['path']),
        'class' => 'photos_edit_thumb'
      ),
      '<div class="photos-edit-title">' . drupal_render($form['photos'][$key]['title']) . '</div><div class="photos-edit-des">' . drupal_render($form['photos'][$key]['des']) . '</div><div class="photos-edit-info">' . drupal_render($form['photos'][$key]['time']) . drupal_render($form['photos'][$key]['pid']) . '</div>'
    );
  }
  if (!$rows) {
    $rows[] = array(array('data' => t('No image'), 'colspan' => '3'));
  }
  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'photos_album_edit')));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Update album cover.
 */
function photos_edit_cover($node, $fid) {
  $pid = db_query('SELECT pid FROM {photos_image} WHERE fid = :fid', array(':fid' => $fid))->fetchField();
  if ($pid == $node->nid) {
    db_query('UPDATE {photos_album} SET fid = :fid WHERE pid = :pid',
      array(':fid' => $fid, ':pid' => $node->nid));
    drupal_set_message(t('Cover successfully set.'));
    $goto = isset($_GET['destination']) ? $_GET['destination'] : 'node/' . $node->nid . '/photos';

    return drupal_goto($goto);
  }
  else {
    drupal_not_found();
  }
}

/**
 * Ajax edit image.
 */
function photos_edit_update($fid = NULL) {
  drupal_add_http_header('Content-Type', 'text/plain;');
  $value = trim($_POST['value']);
  $id = check_plain($_POST['id']);
  if (strstr($id, 'title')) {
    $switch = 'title';
    $fid = str_replace('photos-image-edit-title-', '', $id);
  }
  elseif (strstr($id, 'des')) {
    $switch = 'des';
    $fid = str_replace('photos-image-edit-des-', '', $id);
  }
  // Validate token and check user image edit permissions.
  if (drupal_valid_token($_POST['token'], 'image_edit_token') && _photos_access('imageEdit', $fid)) {
    switch ($switch) {
      case 'title':
        db_update('photos_image')
          ->fields(array(
            'title' => $value
          ))
          ->condition('fid', $fid)
          ->execute();
        echo check_plain($value);
      break;
      case 'des':
        db_update('photos_image')
          ->fields(array(
            'des' => $value
          ))
          ->condition('fid', $fid)
          ->execute();
        echo check_plain($value);
      break;
      case 'del':
        // echo photos_file_del($fid, 0, 1);
    }
  }
}

/**
 * Ajax edit image load text.
 */
function photos_edit_update_load($token = NULL) {
  drupal_add_http_header('Content-Type', 'text/plain;');
  $id = check_plain($_POST['id']);
  if (strstr($id, 'title')) {
    $switch = 'title';
    $fid = str_replace('photos-image-edit-title-', '', $id);
  }
  elseif (strstr($id, 'des')) {
    $switch = 'des';
    $fid = str_replace('photos-image-edit-des-', '', $id);
  }
  // Validate token and check user image edit permissions.
  if (drupal_valid_token($token, 'image_edit_token') && _photos_access('imageEdit', $fid)) {
    switch ($switch) {
      case 'title':
        $value = db_query("SELECT title FROM {photos_image} WHERE fid = :fid", array(':fid' => $fid))->fetchField();
        echo $value;
      break;
      case 'des':
        $value = db_query("SELECT des FROM {photos_image} WHERE fid = :fid", array(':fid' => $fid))->fetchField();
        echo $value;
      break;
    }
  }
}

/**
 * Delete photo.
 */
function photos_edit_delete($fid) {
  if (module_exists('colorbox')) {
    // Dispaly form in modal popup.
    $confirm_delete_form = drupal_get_form('photos_edit_confirm_delete', $fid);
    print drupal_render($confirm_delete_form);
  }
  else {
    // Render full page.
    return drupal_get_form('photos_edit_confirm_delete', $fid);
  }
}

/**
 * Confirm delete photo.
 */
function photos_edit_confirm_delete($form, &$form_state, $fid) {
  $type = isset($_GET['type']) ? check_plain($_GET['type']) : '';
  $form['fid'] = array(
    '#type' => 'value',
    '#value' => $fid,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );
  if ($type <> 'sub_album') {
    $description = t('This photo will be deleted from this gallery and all sub galleries along with all comments, title and description.');
    $remove = t('Delete');
  }
  else {
    $description = t('This photo will be moved out of this sub gallery only.');
    $remove = t('Move out');
  }
  $path = array('path' => 'photos/image/' . $fid);
  return confirm_form($form, t('Confirm and delete this photo.'), $path, $description, $remove, t('Cancel'));
}

/**
 * Submit confirm delete photo.
 */
function photos_edit_confirm_delete_submit($form, &$form_state) {
  $fid = $form_state['values']['fid'];
  $type = isset($form_state['values']['type']) ? $form_state['values']['type'] : '';

  if ($type <> 'sub_album') {
    // Remove from search index.
    if (module_exists('search')) {
      search_reindex($fid, 'photos');
    }
    // Delete image.
    $v = photos_file_del($fid, 0, 1);
    if (isset($_GET['pid']) && intval($_GET['pid']) == $_GET['pid']) photos_set_count('node_album', $_GET['pid']);
    if (isset($_GET['uid']) && intval($_GET['uid']) == $_GET['uid']) photos_set_count('user_image', $_GET['uid']);
  }
  else {
    // Remove from sub album.
    $v = db_delete('photos_node')
      ->condition('fid', $fid)
      ->execute();
    if (isset($_GET['nid']) && intval($_GET['nid']) == $_GET['nid']) photos_set_count('node_node', $_GET['nid']);
  }
  if (isset($_GET['go']) && $_GET['go'] == 1) {
    drupal_add_http_header('Content-Type:', 'text/plain;');
    echo $v;
  }
  elseif ($v) {
    if ($type == 'sub_album') {
      drupal_set_message(t('Delete success'));
    }
    else {
      drupal_set_message(t('Delete success'));
    }
    $form_state['redirect'] = $_GET['destination'];
    return;
  }
  else {
    drupal_set_message(t('Delete failure'));
    $form_state['redirect'] = $_GET['destination'];
    return;
  }
}

/**
 * Move image to sub album.
 */
function photos_edit_to_sub($fid) {
  global $user;
  $photos_to_sub_form = drupal_get_form('_photos_to_sub', $fid);
  $content = drupal_render($photos_to_sub_form);
  $content .= theme('pager');
  print theme('photos_print', array('content' => $content));
}

/**
 * Move to sub album form.
 */
function _photos_to_sub($form, &$form_state, $fid) {
  $select_type = _photos_select_sub_album();
  if ($select_type[0]) {
    $form['title']['#markup'] = '<h2>' . t('Please select sub-album') . ': ' . '</h2>';
    $query = db_select('photos_node', 'p')
      ->fields('p', array('nid'))
      ->condition('fid', $fid);
    $result = $query->execute();
    $select_sub = array();
    foreach ($result as $sub) {
      $select_sub[] = $sub->nid;
    }
    if (!isset($select_sub[0])) $select_sub[] = 0;
    $query = db_select('node', 'n')->extend('PagerDefault');
    $query->fields('n', array('nid', 'title'))
      ->condition('n.type', $select_type, 'IN')
      ->condition('n.nid', $select_sub, 'NOT IN')
      ->limit(50);
    $result = $query->execute();
    $form['sub']['#tree'] = TRUE;
    $true = FALSE;
    foreach ($result as $node) {
      $form['sub'][$node->nid] = array(
        '#type' => 'checkbox',
        '#title' => l($node->title, 'node/' . $node->nid, array('attributes' => array('target' => '_blank'))),
      );
      $true = TRUE;
    }
    if ($true) {
      $form['fid'] = array(
        '#type' => 'value',
        '#value' => $fid
      );
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Send confirmation'),
        '#submit' => array('_photos_to_sub_submit')
      );
    }
    else {
      $form['help']['#markup'] = t('There are no additional sub albums available.');
    }
  }
  else {
    $form['help']['#markup'] = t('Sub-album feature is not turned on.');
  }

  return $form;
}

/**
 * Submit moving images to sub-album(s).
 */
function _photos_to_sub_submit($form, &$form_state) {
  if (!$form_state['values']['fid']) return;
  $query = db_insert('photos_node')->fields(array('nid', 'fid'));
  $nids = array();
  foreach ($form_state['values']['sub'] as $key => $sub) {
    if ($sub) {
      $query->values(array(
        'nid' => $key,
        'fid' => $form_state['values']['fid']
      ));
      $nids[] = $key;
    }
  }
  if (!empty($nids)) {
    $query->execute();
    foreach ($nids as $nid) {
      photos_set_count('node_node', $nid);
    }
    $count = count($nids);
    $msg = format_plural($count,
      'Successfully sent to 1 sub-album.',
      'Successfully sent to @count sub-albums.');
    drupal_set_message($msg);
  }
  $redirect = array('photos/image/' . $form_state['values']['fid']);
  $form_state['redirect'] = $redirect;
}
