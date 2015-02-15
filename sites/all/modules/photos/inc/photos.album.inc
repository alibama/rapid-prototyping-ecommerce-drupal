<?php
/**
 * @file
 * Handles album views and content.
 */

/**
 * @see photos_albumview.tpl.php
 *
 * Album view.
 */
function photos_album_page($node) {
  $build = array();
  $order = explode('|', (isset($node->album['imageorder']) ? $node->album['imageorder'] : variable_get('photos_display_imageorder', 'timestamp|desc')));
  $order = _photos_order_value_change($order[0], $order[1]);
  $limit = isset($node->album['full_viewnum']) ? $node->album['full_viewnum'] : variable_get('photos_display_viewpager', 10);
  $column = isset($_GET['field']) ? $_GET['field'] : '';
  $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
  $term = _photos_order_value($column, $sort, $limit, $order);
  $query = db_select('file_managed', 'f')->extend('PagerDefault');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('users', 'u', 'u.uid = f.uid');
  $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'))
    ->fields('p')
    ->fields('u', array('uid', 'name'))
    ->condition('p.pid', $node->nid)
    ->orderBy($term['order']['column'], $term['order']['sort'])
    ->limit($term['limit']);
  $result = $query->execute();

  $com = variable_get('photos_comment', 0);
  $vote = variable_get('photos_vote', 0);
  $edit = node_access('update', $node);
  $del = node_access('delete', $node);
  $style_name = isset($node->album['list_imagesize']) ? $node->album['list_imagesize'] : variable_get('photos_display_list_imagesize', 'thumbnail');
  // Necessary when upgrading from D6 to D7.
  $image_styles = image_style_options(FALSE);
  if (!isset($image_styles[$style_name])) {
    $style_name = variable_get('photos_display_list_imagesize', 'medium');
  }
  foreach ($result as $data) {
    $image = photos_get_info(0, $data);
    $image->title = check_plain($image->title);
    $image->des = check_plain($image->des);
    $title = $image->title;
    $image->view = theme('photos_imagehtml', array('style_name' => $style_name, 'image' => $image));
    $image->url = url('photos/image/' . $image->fid);
    if ($com) {
      $image->links['comment'] = theme('photos_comment_count', array('comcount' => $image->comcount, 'url' => $image->url));
    }
    if ($image->count) {
      $image->links['count'] = format_plural($image->count, '!cou visit', '!cou visits', array('!cou' => $image->count));
    }
    $image->links['info'] = t('Uploaded on !time by !name', array('!name' => $image->name, '!time' => format_date($image->timestamp, 'small')));
    $image->class = array(
      'title_class' => '',
      'des_class' => '',
    );
    $image->id = array(
      'des_edit' => '',
      'title_edit' => ''
    );
    if ($edit) {
      $destination = drupal_get_destination();
      $image->ajax['edit_url'] = $image->url . '/update';
      $image->ajax['edit_link'] = l(t('Edit'), 'photos/image/' . $image->fid . '/edit', array(
        'query' => array(
          'destination' => $destination['destination'],
          'pid' => $node->nid,
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
      $image->links['cover'] = l(t('Set to Cover'), 'node/' . $image->pid . '/photos/cover/' . $image->fid, array('query' => $destination));
    }
    $image->ajax['del_id'] = '';
    if ($del) {
      $image->ajax['del_id'] = 'id="photos_ajax_del_' . $image->fid . '"';
      $destination = drupal_get_destination();
      $image->ajax['del_link'] = l(t('Delete'), 'photos/image/' . $image->fid . '/delete', array(
        'query' => array(
          'destination' => $destination['destination'],
          'pid' => $node->nid,
          'uid' => $image->uid
        ),
        'attributes' => array(
          'class' => array('colorbox-load', 'photos-edit-delete')
        )
      ));
    }
    if ($vote) {
      $image->links['vote'] = theme('photos_vote', array('fid' => $image->fid));
    }
    $album['view'][] = theme('photos_imageview', array('image' => $image, 'type' => 'list'));
  }
  if (isset($album['view'][0])) {
    $album['access']['edit'] = $edit;
    $album['node_edit_url'] = l(t('Album settings'), 'node/' . $node->nid . '/edit');
    $album['image_management_url'] = l(t('Images Management'), 'node/' . $node->nid . '/photos');
    $album['album_url'] = url('photos/album/' . $node->nid);
    $album['links'] = _photos_order_link('photos/album/' . $node->nid, 0, 0, 1);
    $cover_style_name = variable_get('photos_cover_imagesize', 'thumbnail');
    if (isset($node->album['cover']['url'])) {
      $album_cover = theme('image_style', array(
        'style_name' => $cover_style_name,
        'path' => $node->album['cover']['url'],
        'alt' => $node->title,
        'title' => $node->title
      ));
      $album['cover'] = $album_cover;
    }
    $v = theme('photos_albumview', array('album' => $album, 'node' => $node));
  }
  else {
    $v = t('Album is empty');
  }
  $node_title = $node->title;
  _photos_breadcrumb(array(l(t('@name\'s albums', array('@name' => $node->name)), 'photos/user/' . $node->uid . '/album'), l($node_title, 'node/' . $node->nid)));
  $page_title = t('Albums: @title', array('@title' => $node_title));
  drupal_set_title($page_title, PASS_THROUGH);
  $v .= theme('pager');

  return $v;
}

/**
 * @see photos_albumview.tpl.php
 *
 * Sub album view.
 */
function photos_sub_album_page($node) {
  $order = explode('|', variable_get('photos_display_imageorder', 'timestamp|desc'));
  $order = _photos_order_value_change($order[0], $order[1]);
  $column = isset($_GET['field']) ? $_GET['field'] : '';
  $sort = isset($_GET['sort']) ? $_GET['sort'] : '';
  $term = _photos_order_value($column, $sort, variable_get('photos_display_viewpager', 10), $order);
  // Override weight sort for sub albums.
  if ($term['order']['column'] == 'p.wid') $term['order']['column'] = 'a.wid';
  $query = db_select('file_managed', 'f')->extend('PagerDefault');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('photos_node', 'a', 'a.fid = f.fid');
  $query->join('node', 'n', 'n.nid = p.pid');
  $query->join('users', 'u', 'u.uid = f.uid');
  $query->addField('n', 'title', 'album_title');
  $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename', 'filesize'))
    ->fields('p')
    ->fields('a')
    ->fields('u', array('uid', 'name'))
    ->condition('a.nid', $node->nid);
  $query->orderBy($term['order']['column'], $term['order']['sort']);
  $query->range(0, $term['limit']);
  $result = $query->execute();

  $com = variable_get('photos_comment', 0);
  $edit = node_access('update', $node);
  $del = node_access('delete', $node);
  $vote = variable_get('photos_vote', 0);
  $style_name = variable_get('photos_display_list_imagesize', 'medium');
  foreach ($result as $data) {
    $image = photos_get_info(0, $data);
    $image->view = theme('photos_imagehtml', array('style_name' => $style_name, 'image' => $image));
    $image->url = url('photos/image/' . $image->fid, array('query' => array('photos_sub' => $image->nid)));
    if ($com) {
      $image->links['comment'] = theme('photos_comment_count', array('comcount' => $image->comcount, 'url' => $image->url));
    }
    if ($image->count) {
      $image->links['count'] = format_plural($image->count, '!cou visit', '!cou visits', array('!cou' => $image->count));
    }
    $image->links['info'] = t('Uploaded by !name on !time to !title', array('!name' => $image->name, '!time' => format_date($image->timestamp, 'small'), '!title' => l($image->album_title, 'photos/album/' . $image->pid)));
    $image->class = array(
      'title_class' => '',
      'des_class' => '',
    );
    $image->id = array(
      'des_edit' => '',
      'title_edit' => ''
    );
    if ($edit) {
      $image->ajax['edit_url'] = $image->url . '/update';
      // $image->links['cover'] = l(t('Set as Cover'), 'node/' . $image->pid . '/photos/cover/' . $image->fid, array('query' => drupal_get_destination()));
      $image->class = array(
        'title_class' => ' jQueryeditable_edit_title',
        'des_class' => ' jQueryeditable_edit_des',
      );
      $image->id = array(
        'des_edit' => ' id="photos-image-edit-des-' . $image->fid . '"',
        'title_edit' => ' id="photos-image-edit-title-' . $image->fid . '"'
      );
    }
    $image->ajax['del_id'] = '';
    if ($del) {
      $image->ajax['del_id'] = 'id="photos_ajax_del_' . $image->fid . '"';
      $image->ajax['del_link'] = l(t('Move out'), 'photos/image/' . $image->fid . '/delete', array(
        'query' => array(
          'destination' => $_GET['q'],
          'type' => 'sub_album',
          'nid' => $node->nid
        ),
        'attributes' => array(
          'class' => 'colorbox-load',
        )
      ));
    }
    if ($vote) {
      $image->links['vote'] = theme('photos_vote', array('fid' => $image->fid));
    }
    $album['view'][] = theme('photos_imageview', array('image' => $image, 'type' => 'list'));
  }
  if (isset($album['view'][0])) {
    $album['node_url'] = url('node/' . $node->nid);
    $album['album_url'] = url('photos/sub_album/' . $node->nid);
    $album['links'] = _photos_order_link('photos/album/' . $node->nid, $node->subalbum['count'], 0, 1);
    $album['pager'] = theme('pager');
    if (isset($node->album['cover']['url'])) {
      $album_cover = theme('image_style', array(
        'style_name' => 'thumbnail',
        'path' => $node->album['cover']['url'],
        'alt' => $node->title,
        'title' => $node->title
      ));
      $album['cover'] = $album_cover;
    }
    $v = theme('photos_albumview', array('album' => $album, 'node' => $node));
  }
  else {
    $v = t('Sub-Album is empty');
  }
  $page_title = t('Sub-Album: @title', array('@title' => $node->title));
  _photos_breadcrumb(array(
    l($node->title, 'node/' . $node->nid)
  ));
  drupal_set_title($page_title);

  return $v;
}

/**
 * Photos album share.
 */
function photos_album_share($node) {
  $type =  $node->type == 'photos' ? 'album' : 'sub_album';
  $content['flash'] = _photos_slide($node);
  $content['html'] = array(
    'image' => $node->album['cover']['url'] ? _photos_l($node->album['cover']['url']) : '',
    'title' => $node->title,
    'link' => url('photos/' . $type . '/' . $node->nid, array('absolute' => TRUE)),
  );

  print theme('photos_print', array('content' => $content, 'type' => 'share'));
}

/**
 * Album slide show.
 */
function photos_album_slide($node) {
  if ($node->album['count'] || $node->subalbum['count']) {
    $type = $node->type == 'photos' ? 'album' : 'sub_album';

    _photos_breadcrumb(array(l(t('@name\'s albums', array('@name' => $node->name)), 'photos/user/' . $node->uid . '/album'), l($node->title, 'photos/' . $type . '/' . $node->nid)));

    $page_title = t('Slide: @title', array('@title' => $node->title));
    drupal_set_title($page_title);

    $share_code = l(t('Share code'), 'photos/' . $type . '/' . $node->nid . '/share', array(
      'query' => array(
        'KeepThis' => 'true',
        'TB_iframe' => 'true',
        'height' => '450',
        'width' => '650'
      ),
      'attributes' => array(
        'title' => t('Share code'),
        'class' => array('colorbox')
      )
    ));
    $v = '<p>' . l(t('Back'), 'photos/' . $type . '/' . $node->nid) . ' | ' . $share_code . '</p>';
    $v .= _photos_slide($node);
  }
  else {
    $v = t('No image');
  }

  return $v;
}

/**
 * Slide show.
 * @todo add support for jQuery and other slide show(s).
 */
function _photos_slide($node) {
  if ($node->album['slide']) {
    list($width, $height) = explode('x', $node->album['slide']);
  }
  else {
    list($width, $height) = explode('x', variable_get('photos_display_slide', '640x480'));
  }
  $type =  $node->type == 'photos' ? 'album' : 'sub_album';

  if (module_exists('dfgallery')) {
    $output = dfgallery_html(array(
      'url' => url('photos/data/' . $type . '/' . $node->nid . '/json.json',
        array('absolute' => TRUE)
      ),
      'width' => $width,
      'height' => $height
    ));
    return $output;
  }
}
