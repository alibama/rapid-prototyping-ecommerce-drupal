<?php
/**
 * @file
 * Handles slideshow integration.
 */

/**
 * Album slideshow page.
 */
function photos_page_slide($account) {
  $v = '';
  if ($account->album['album']['count'] && $account->album['image']['count']) {
    list($width, $height) = explode('x', variable_get('photos_display_slide', '640x480'));
    if (module_exists('dfgallery')) {
      $v = dfgallery_html(array('url' => url('photos/data/user/' . $ac->uid . '/json.json', array('absolute' => TRUE)), 'width' => $width, 'height' => $height));
    }
  }
  else {
    $v = photos_page_title($ac, t('has not yet create an album'));
  }

  return $v;
}

/**
 * All user photos slideshow.
 */
function photos_data_user_slide($account) {
  drupal_add_http_header('Content-Type:', 'text/plain;');
  drupal_add_http_header('Cache-Control:', 'no-cache, must-revalidate');
  drupal_add_http_header('Expires:', 'Sat, 26 Jul 1997 05:00:00 GMT');
  $query = db_select('file_managed', 'f');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('node', 'n', 'p.pid = n.nid');
  $query->join('photos_album', 'a', 'a.pid = n.nid');
  $query->fields('f', array('fid', 'uri', 'filemime', 'timestamp', 'filename', 'filesize'))
    ->fields('n', array('nid', 'title'))
    ->condition('n.uid', $account->uid)
    ->condition('a.count', '0', '>')
    ->orderBy('f.fid', 'DESC')
    ->limit(500);
  $result = $query->execute();
  $thumb = variable_get('photos_slide_show_thumb', 'thumbnail');
  $view = variable_get('photos_slide_show_view', 'large');
  $true = FALSE;
  $t = array();
  foreach ($result as $image) {
    $t[$image->nid]['node'] = array(
      'title' => $image->title,
      'link' => url('photos/album/' . $image->nid),
    );
    $images = photos_get_info(0, $image);
    $t[$image->nid]['image'][] = array(
      'title' => $images->filename,
      'timestamp' => $images->timestamp,
      'thumbnail' => _photos_l($image->uri, $thumb),
      'image' => _photos_l($image->uri, $view)
    );
    $true = TRUE;
  }
  if ($_GET['type'] != 'xml' && $true) {
    foreach ($t as $key => $v) {
      $album[] = array(
        'properties' => $t[$key]['node'],
        'images' => $t[$key]['image']
      );
    }
    if (variable_get('photos_slide_music', FALSE)) {
      $setting['config']['theme']['config_theme_music'] = variable_get('photos_slide_music', FALSE);
    }
    if (module_exists('dfgallery')) {
      print dfgallery_json($album, $setting);
    }
  }
}

/**
 * Sub-album slideshow page.
 */
function photos_data_sub_block_slide($node, $op) {
  drupal_add_http_header('Content-Type:', 'text/plain;');
  drupal_add_http_header('Cache-Control:', 'no-cache, must-revalidate');
  drupal_add_http_header('Expires:', 'Sat, 26 Jul 1997 05:00:00 GMT');
  switch ($op) {
    case 'block_new':
      $query = db_select('file_managed', 'f');
      $query->join('photos_image', 'p', 'p.fid = f.fid');
      $query->join('node', 'n', 'n.nid = p.pid');
      $query->join('users', 'u', 'u.uid = f.uid');
      $query->join('photos_node', 'a', 'a.fid = p.fid');
      $query->fields('f', array('fid', 'uri', 'filemime', 'timestamp', 'filename', 'filesize'))
        ->fields('n', array('title'))
        ->fields('u', array('uid', 'name'))
        ->fields('p', array('pid', 'comcount', 'count'))
        ->condition('a.nid', $node->nid)
        ->orderBy('f.fid', 'DESC')
        ->limit(3);
      $result = $query->execute();
      $i = 0;
      foreach ($result as $images) {
        $image = photos_get_info(0, $images);
        $t = array();
        if (is_array($image->thumb)) {
          foreach ($image->thumb as $key => $thumb) {
            $t['url'] = _photos_l($thumb);
            break;
          }
        }
        $t['filename'] = $image->filename;
        $t['link'] = _photos_l('photos/image/' . $image->fid);
        $array[] = $t;
        ++$i;
      }
      $array['count'] = $i;
  }
  if ($_GET['type'] != 'xml' && $array['count']) {
    print _photos_json('en', $array);
  }
}

/**
 * Album slideshow data.
 */
function photos_data_album($node) {
  drupal_add_http_header('Content-Type:', 'text/plain;');
  drupal_add_http_header('Cache-Control:', 'no-cache, must-revalidate');
  drupal_add_http_header('Expires:', 'Sat, 26 Jul 1997 05:00:00 GMT');
  if ($node->type == 'photos') {
    $order = explode('|', ($node->album['imageorder'] ? $node->album['imageorder'] : variable_get('photos_display_imageorder', 'timestamp|desc')));
    $order = _photos_order_value_change($order[0], $order[1]);
    $column = isset($_GET['field']) ? $_GET['field'] : '';
    $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
    $limit = 100;
    $term = _photos_order_value($column, $sort, $limit, $order);
    $query = db_select('file_managed', 'f');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->fields('f', array('fid', 'uri', 'filemime', 'timestamp', 'filename', 'filesize'))
      ->condition('p.pid', $node->nid)
      ->orderBy($term['order']['column'], $term['order']['sort'])
      ->limit($term['limit']);
    $result = $query->execute();
  }
  else {
    $order = explode('|', variable_get('photos_display_imageorder', 'timestamp|desc'));
    $order = _photos_order_value_change($order[0], $order[1]);
    $column = isset($_GET['field']) ? $_GET['field'] : '';
    $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
    $limit = 100;
    $term = _photos_order_value($column, $sort, $limit, $order);
    $query = db_select('file_managed', 'f');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->join('photos_node', 'a', 'a.fid = p.fid');
    $query->fields('f', array('fid', 'uri', 'filemime', 'timestamp', 'filename', 'filesize'))
      ->condition('a.nid', $node->nid)
      ->orderBy($term['order']['column'], $term['order']['sort'])
      ->limit($term['limit']);
    $result = $query->execute();
  }
  $thumb = variable_get('photos_slide_show_thumb', 0);
  $view = variable_get('photos_slide_show_view', 0);
  $true = FALSE;
  foreach ($result as $images) {
    $image = photos_get_info(0, $images);
    $array[] = array(
      'title' => $image->filename,
      'timestamp' => $image->timestamp,
      'thumbnail' => _photos_l($image->thumb[$thumb]),
      'image' => _photos_l($image->thumb[$view])
    );
    $true = TRUE;
  }
  if ($true) {
    $albums = array(
      array(
        'properties' => array(
          'title' => check_plain($node->title),
          'description' => check_plain($node->teaser), // 相册描述
          'icon' => $node->album['cover']['url'], // 相册封面
        ),
        'images' => $array,
      )
    );
    if (variable_get('photos_slide_music', FALSE)) {
      $setting['config']['theme']['config_theme_music'] = $node->album['slide_music'] ? $node->album['slide_music'] : variable_get('photos_slide_music', FALSE);
    }
    print dfgallery_json($albums, $setting);
  }
}

// Not called?
function _photos_thumb_json($image) {
  $t['file']['fid'] = $image->fid;
  $t['file']['filename'] = $image->filename;
  $t['user']['link'] = _photos_l('user/' . $image->uid);
  $t['user']['name'] = $image->name;
  $t['file']['album_title'] = $image->title;
  $t['file']['album_link'] = _photos_l('photos/album/' . $image->pid);
  if ($t['file']['des']) $t['file']['des'] = $image->des;
  $t['original'] = _photos_l($image->original);
  if (is_array($image->thumb)) {
    foreach ($image->thumb as $key => $thumb) {
      $t['thumb'][$key] = _photos_l($thumb);
    }
  }
  return $t;
}