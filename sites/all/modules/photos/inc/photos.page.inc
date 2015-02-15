<?php
/**
 * @file
 * Handles default page view(s) and content.
 */

/**
 * Default page, list recent photos and galleries.
 */
function photos_page_default() {
  global $user;
  $content = array();
  if ($user->uid && user_access('create photo')) {
    $val = _photos_block_image('user', 5, 'photos/user/' . $user->uid . '/image', $user->uid);
    $content['user']['image'] = $val[0];
    $val = _photos_block_album('user', 5, 'photos/user/' . $user->uid . '/album', $user->uid);
    $content['user']['album'] = $val[0];
  }
  $content['site']['image'] = _photos_block_image('latest', 5, 'photos/image');
  $content['site']['album'] = _photos_block_album('latest', 5, 'photos/album');

  return theme('photos_default', array('content' => $content));
}

/**
 * Album photos page.
 */
function photos_page_album($account = FALSE) {
  $build = array();
  $output = '';
  if (isset($account->uid) && $account->uid <> 0) {
    global $user;
    if ($user->uid == $account->uid) {
      // @todo add support for admin to re-arrange albums.
      $output = l(t('Re-arrange albums'), 'photos/user/' . $account->uid . '/album-sort');
    }
    $query = db_select('node', 'n')
      ->extend('PagerDefault');
    $query->join('photos_album', 'p', 'p.pid = n.nid');
    $query->fields('n', array('nid'));
    $query->condition('n.uid', $account->uid);
    $query->orderBy('p.wid', 'ASC');
    $query->orderBy('n.nid', 'DESC');
    $query->limit(10);
    $result = $query->execute();

    _photos_breadcrumb(array(l(t('Albums'), 'photos/album')));
  }
  else {
    $query = db_select('node', 'n')
      ->extend('PagerDefault');
    $query->join('photos_album', 'p', 'p.pid = n.nid');
    $query->fields('n', array('nid'));
    $query->orderBy('n.nid', 'DESC');
    $query->limit(10);
    $query->addTag('node_access');
    $result = $query->execute();
  }
  foreach ($result as $node) {
    $node = node_load($node->nid);
    $node_view = node_view($node, 'full');
    $output .= drupal_render($node_view);
  }
  if ($output) {
    $output .= theme('pager');
  }
  else {
    if ($account <> FALSE) {
      $output .= t('@name has not created an album yet.', array('@name' => $account->name));
    }
    else {
      $output .= t('No albums have been created yet.');
    }
  }

  return $output;
}

/**
 * Latest images page.
 */
function photos_page_image($account = FALSE, $fid = NULL) {
  global $user;
  if ($account && is_numeric($account)) {
    $account = user_load($account);
  }
  $build = array();
  $order = explode('|', variable_get('photos_display_imageorder', 'timestamp|desc'));
  $order = _photos_order_value_change($order[0], $order[1]);
  $column = (isset($_GET['field'])) ? check_plain($_GET['field']) : 0;
  $sort = (isset($_GET['sort'])) ? check_plain($_GET['sort']) : 0;
  $term = _photos_order_value($column, $sort, variable_get('photos_display_viewpager', 10), $order);
  if (isset($account->uid) && $fid) {
    $inc = module_load_include('inc', 'photos', 'inc/photos.image');
    return photos_image_page($fid, $account->uid);
  }
  elseif (isset($account->uid)) {
    $query = db_select('file_managed', 'f')
      ->extend('PagerDefault');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->join('node', 'n', 'n.nid = p.pid');
    $query->join('users', 'u', 'u.uid = f.uid');
    $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'))
      ->fields('p')
      ->fields('u', array('uid', 'name'));
    $query->addField('n', 'title', 'node_title');
    $query->condition('f.uid', $account->uid);
    $query->orderBy($term['order']['column'], $term['order']['sort']);
    $query->limit($term['limit']);
    $result = $query->execute();

    $slideshow = '';
    if (module_exists('dfgallery')) {
      $slideshow = l(t('Slideshow'), 'photos/user/' . $account->uid . '/slide');
    }
    $album['links'] = _photos_order_link('photos/user/' . $account->uid . '/image', $account->album['image']['count'], $slideshow, 1);
    _photos_breadcrumb(array(l(t('Images'), 'photos/image')));
  }
  else {
    $query = db_select('file_managed', 'f')
      ->extend('PagerDefault');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->join('node', 'n', 'n.nid = p.pid');
    $query->join('users', 'u', 'u.uid = f.uid');
    $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'))
      ->fields('p')
      ->fields('u', array('uid', 'name'));
    $query->addField('n', 'title', 'node_title');
    $query->orderBy($term['order']['column'], $term['order']['sort']);
    $query->limit($term['limit']);
    $query->addTag('node_access');
    $result = $query->execute();

    $album['links'] = _photos_order_link('photos/image', photos_get_count('site_image'), 0, 1);
  }
  $com = variable_get('photos_comment', 0);
  $edit = 0;
  if (isset($account->uid) && isset($user->uid) && $account->uid == $user->uid) {
    $edit = 1;
  }
  $vote = variable_get('allowed to vote', 0);
  $style_name = variable_get('photos_display_list_imagesize', FALSE);
  foreach ($result as $data) {
    $image = photos_get_info(0, $data);
    $image->title = check_plain($image->title);
    $image->des = check_plain($image->des);
    $image->view = theme('photos_imagehtml', array('style_name' => $style_name, 'image' => $image));
    $image->url = url('photos/image/' . $image->fid);
    if ($com) {
      $image->links['comment'] = theme('photos_comment_count', array('comcount' => $image->comcount, 'url' => $image->url));
    }
    if (isset($image->count)) {
      $image->links['count'] = format_plural($image->count, '!cou visit', '!cou visits', array('!cou' => $image->count));
    }
    // @todo optimize username.
    // @note user_format_name() in drupal 8.
    if (!isset($account->uid) || !empty($image->uid) && $image->uid <> $account->uid) {
      $account = user_load($image->uid);
    }
    $name = format_username($account);
    $image->links['info'] = t('Uploaded on !time by !name in !title', array(
      '!name' => $name,
      '!time' => format_date($image->timestamp, 'small'),
      '!title' => l($image->node_title, 'photos/album/' . $image->pid)
    ));
    $image->class = array(
      'title_class' => '',
      'des_class' => '',
    );
    $image->id = array(
      'des_edit' => '',
      'title_edit' => ''
    );
    $image->ajax['del_id'] = '';
    if ($edit) {
      $image->ajax['edit_url'] = $image->url . '/update';
      $image->ajax['edit_link'] = l(t('Edit'), 'photos/image/' . $image->fid . '/edit', array(
        'query' => array(
          'destination' => $_GET['q'],
          'pid' => $image->pid,
          'uid' => $image->uid
        ),
        'attributes' => array(
          'class' => array('colorbox-load', 'photos-edit-edit')
        )
      ));
      $image->class = array(
        'title_class' => ' jQueryeditable_edit_title',
        'des_class' => ' jQueryeditable_edit_des',
      );
      $image->id = array(
        'des_edit' => ' id="photos-image-edit-des-' . $image->fid . '"',
        'title_edit' => ' id="photos-image-edit-title-' . $image->fid . '"'
      );
      $image->ajax['del_id'] = 'id="photos_ajax_del_' . $image->fid . '"';
      $image->ajax['del_link'] = l(t('Delete'), 'photos/image/' . $image->fid . '/delete', array(
        'query' => array(
          'destination' => $_GET['q'],
          'pid' => $image->pid,
          'uid' => $image->uid
        ),
        'attributes' => array(
          'class' => array('photos-edit-delete'),
          'alt' => 'photos_ajax_del_' . $image->fid
        )
      ));
      $image->links['cover'] = l(t('Set to Cover'), 'node/' . $image->pid . '/photos/cover/' . $image->fid, array('query' => drupal_get_destination()));
    }
    if ($vote) {
      $image->links['vote'] = theme('photos_vote', array('fid' => $image->fid));
    }
    $album['view'][] = theme('photos_imageview', array('image' => $image, 'type' => 'list'));
  }
  if (isset($album['view'][0])) {
    $v = theme('photos_albumview', array('album' => $album, 'node' => NULL));
  }
  else {
    if ($account <> FALSE) {
      $v = t('@name has not uploaded any images yet.', array('@name' => $account->name));
    }
    else {
      $v = t('No images have been uploaded yet.');
    }
  }
  $v .= theme('pager');

  return $v;
}

/**
 * Share photos.
 */
function photos_share() {
  global $user;
  $output = '';
  $order = explode('|', variable_get('photos_display_imageorder', 'timestamp|desc'));
  $order = _photos_order_value_change($order[0], $order[1]);
  $column = (isset($_GET['field'])) ? check_plain($_GET['field']) : 0;
  $sort = (isset($_GET['sort'])) ? check_plain($_GET['sort']) : 0;
  $term = _photos_order_value($column, $sort, 20, $order);
  if (arg(2) != 'all') {
    $query = db_select('file_managed', 'f')
      ->extend('PagerDefault');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->fields('f', array('uid', 'fid', 'uri', 'filemime', 'filename'))
      ->fields('p', array('title'));
    $query->condition('f.uid', $user->uid);
    $query->orderBy($term['order']['column'], $term['order']['sort']);
    $query->limit($term['limit']);
    $result = $query->execute();

    $images['menu'] = _photos_order_link('photos/share', 0, 0, 1);
  }
  else {
    $query = db_select('file_managed', 'f')
      ->extend('PagerDefault');
    $query->join('photos_image', 'p', 'p.fid = f.fid');
    $query->join('users', 'u', 'u.uid = f.uid');
    $query->fields('f', array('fid', 'uri', 'filemime', 'filename', 'timestamp', 'filesize'))
      ->fields('p', array('title'))
      ->fields('u', array('uid', 'name'));
    $query->condition('f.uid', $user->uid, '<>');
    $query->orderBy($term['order']['column'], $term['order']['sort']);
    $query->limit($term['limit']);
    $result = $query->execute();

    $images['menu'] = _photos_order_link('photos/share/all', 0, 0, 1);
  }
  $style_name = variable_get('photos_thumb_imagesize', 'thumbnail');
  foreach ($result as $data) {
    $image = photos_get_info(0, $data);
    if (is_array($image->thumb)) {
      foreach ($image->thumb as $key => $thumb) {
        $thumbs[$thumb] = _photos_l($image->uri, $key);
      }
    }
    $username = '';
    if (isset($image->name) && isset($image->uid)) {
      $username = t('By: !name', array('!name' => l($image->name, 'photos/user/' . $image->uid . '/image')));
    }
    $share[] = array(
      'link' => url('photos/image/' . $image->fid, array('absolute' => TRUE)),
      'fid' => $image->fid,
      'filename' => $image->filename,
      'title' => $image->title,
      'thumbs' => $thumbs,
      'username' => $username,
      'view' => theme('photos_imagehtml', array('style_name' => $style_name, 'image' => $image))
    );
  }
  $images['links'] = l(t('« Back'), 'photos') . l(t('My images'), 'photos/share') . l(t('All the shared images'), 'photos/share/all') ;
  $images['data'] = isset($share[0]['fid']) ? $share : '';
  $output .= theme('photos_share', array('images' => $images, 'type' => 'image'));
  $output .= theme('pager');

  print $output;
}
