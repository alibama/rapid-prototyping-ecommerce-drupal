<?php
/**
 * @file
 * Handles image page view(s) and content.
 */

/**
 * Image page.
 */
function photos_image_page($fid, $uid = NULL) {
  if (!is_numeric($fid)) {
    drupal_not_found();
    drupal_exit();
  }
  global $user;
  $query = db_select('file_managed', 'f');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('photos_album', 'a', 'p.pid = a.pid');
  $query->join('node', 'n', 'n.nid = p.pid');
  $query->join('users', 'u', 'u.uid = f.uid');
  $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename'))
    ->fields('p')
    ->fields('a', array('data'))
    ->fields('u', array('uid', 'name'));
  $query->condition('p.fid', $fid);
  $image = $query->execute()->fetchObject();

  if (!$image) return drupal_not_found();
  $image = photos_get_info(0, $image);
  $node = node_load($image->pid);
  if (_photos_access('imageEdit', $node)) {
    $image->ajax['edit_url'] = url('photos/image/' . $image->fid . '/update');
    if (_photos_select_sub_album()) {
      $image->links['to_sub'] = l(t('Add to sub-album...'), 'photos/image/' . $image->fid . '/to_sub', array(
        'attributes' => array(
          'class' => array('colorbox')
        )
      ));
    }
    $image->links['cover'] = l(t('Set to Cover'), 'node/' . $image->pid . '/photos/cover/' . $fid, array(
      'query' => drupal_get_destination())
    );
  }
  $image->class = array(
    'title_class' => '',
    'des_class' => '',
  );
  $image->id = array(
    'des_edit' => '',
    'title_edit' => ''
  );
  if (_photos_access('imageEdit', $node)) {
    $image->ajax['edit_link'] = l(t('Edit'), 'photos/image/' . $image->fid . '/edit', array(
      'query' => array(
        'destination' => 'photos/image/' . $image->fid
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
  }
  if (_photos_access('imageDelete', $node)) {
    $image->ajax['del_link'] = l(t('Delete'), 'photos/image/' . $image->fid . '/delete', array(
      'query' => array(
        'destination' => 'node/' . $image->pid
      ),
      'attributes' => array(
        'class' => array('colorbox-load', 'photos-edit-delete')
      )
    ));
  }
  if (variable_get('photos_vote', 0)) {
    $image->vote = theme('photos_vote', array('fid' => $fid));
  }
  if (variable_get('photos_comment', 0)) {
    $image->links['comment'] = theme('photos_comment_count', array('comcount' => $image->comcount));
  }
  if ($uid) {
    // User images.
    $pager_type = 'uid';
    $pager_id = $uid;
  }
  elseif (isset($_GET['photos_sub'])) {
    // Sub-album images.
    $pager_type = 'sub';
    $pager_id = (int)$_GET['photos_sub'];
  }
  else {
    // Album images.
    $pager_type = 'pid';
    $pager_id = $image->pid;
  }
  $data = unserialize($image->data);
  $style_name = isset($data['view_imagesize']) ? $data['view_imagesize'] : variable_get('photos_display_view_imagesize', 'large');
  // Necessary when upgrading from D6 to D7.
  $image_styles = image_style_options(FALSE);
  if (!isset($image_styles[$style_name])) {
    $style_name = variable_get('photos_display_view_imagesize', 'large');
  }
  // Display all sizes link to share code?
  $all_sizes_link = variable_get('photos_print_sizes', 0);
  if ($all_sizes_link < 2) {
    // Display full page or colorbox.
    $colorbox = array();
    if ($all_sizes_link == 1) {
      $colorbox = array(
        'query' => array(
          'iframe' => 'true',
          'height' => 650,
          'width' => 850
        ),
        'attributes' => array(
          'class' => array('colorbox-load')
        )
      );
    }
    $image->links['more'] = l(t('All sizes'), 'photos/zoom/' . $fid, $colorbox);
  }
  $image->links['pager'] = photos_image_pager($fid, $pager_id, $pager_type);
  $image->view = theme('photos_imagehtml', array('style_name' => $style_name, 'image' => $image));
  $image->comment['view'] = _photos_comment($fid, $image->comcount, $node);
  if (!variable_get('photos_image_count', 0)) {
    $count = 1;
    db_update('photos_image')
      ->fields(array('count' => $count))
      ->expression('count', 'count + :count', array(':count' => $count))
      ->condition('fid', $fid)
      ->execute();
  }
  drupal_set_title($image->title);
  $image->title = check_plain($image->title);
  $image->des = check_plain($image->des);
  if ($pager_type == 'sub') {
    // Home >> Node Title >> Sub Album >> Image
    $sub_album = node_load($pager_id);
    $sub_title = t('Sub-Album: @title', array('@title' => $sub_album->title));
    _photos_breadcrumb(array(
      l($sub_album->title, 'node/' . $sub_album->nid),
      l($sub_title, 'photos/sub_album/' . $sub_album->nid),
      $image->title
    ));
  }
  else {
    _photos_breadcrumb(array(
      l(t('Images'), 'photos/image'),
      l(t('@name\'s images', array('@name' => $image->name)),
      'photos/user/' . $image->uid . '/image'),
      l($node->title, 'photos/album/' . $image->pid),
      $image->title
    ));
  }
  $GLOBALS['photos'][$image->fid . '_pid'] = $image->pid;

  return theme('photos_imageview', array('image' => $image, 'type' => 'view'));
}

/**
 * Photos image vote.
 * photos/image/%/vote
 */
function photos_image_vote($fid) {
  global $user;
  if (user_access('allowed to vote')) {
    switch (arg(4)) {
      case 'up':
        $value = 1;
      break;
      case 'down':
        $value = -1;
      break;
      default:
        exit();
    }
    $vote = array(
      'entity_type' => 'image',
      'entity_id' => $fid,
      'tag' => 'vote',
      'value_type' => 'points',
      'uid' => $user->uid,
      'value' => $value
    );
    $t = votingapi_set_votes($vote);
    if ($t['image'][$fid][0]['value']) {
      $x['sum'] = $t['image'][$fid][2]['value'];
      $x['average'] = $t['image'][$fid][1]['value'];
      $x['count'] = $t['image'][$fid][0]['value'];
    }
  }
  if (!isset($_GET['destination'])) {
    drupal_add_http_header('Content-Type:', 'text/plain;');
    if ($x['count']) {
      foreach ($x as $key => $val) {
        $array[] = '"' . $key . '":' . $val;
      }
      echo '{' . implode(',', $array) . '}';
    }
    else {
      echo '{"count": 0}';
    }
  }
  else {
    if ($x['count']) {
      drupal_set_message(t('Vote success.'));
    }
    else {
      drupal_set_message(t('Vote failure.'));
    }
    return drupal_goto();
  }
}

/**
 * Output image.
 */
function photos_image_get($fid, $thumb) {
  // @todo update $thumb to $style_name.
  $filepath = str_replace('photos/get/' . $fid . '/' . $thumb . '/', '', $_GET['q']);
  if (!is_file($filepath)) {
    if ($thumb != 'original' && ($thumb = _photos_get_thumbname($thumb))) {
      $str = "/thumb_{$fid}.*?thumb_/i";
      $original = preg_replace($str, '', $filepath);
      // deprecated: && ($filepath = photos_get_thumb($original, $fid, $thumb, 1))
      if (is_file($original))
        $true = 1;
    }
    if (!$true) {
      drupal_add_http_header('Content-Type:', 'text/plain;');
      echo t('Image does not exist');
      drupal_exit();
    }
  }
  $image = end(explode('.', $_GET['q']));
  header('Content-type: image/' . $image);
  if ($fd = @fopen($filepath, 'rb')) {
    while (!feof($fd)) {
      print fread($fd, 1024);
    }
    fclose($fd);
    drupal_exit();
  }
  drupal_add_http_header('Content-Type:', 'text/plain;');
  echo t('Image does not exist');
  drupal_exit();
}

/**
 * Comments on single picture.
 */
function _photos_comment($fid, $com_count, $node) {
  global $user;
  $output = array();
  if (module_exists('comment') && user_access('access comments')) {
    if ($com_count && $node->comment <> 0 || user_access('administer comments')) {
      $mode = variable_get('comment_default_mode_' . $node->type, COMMENT_MODE_THREADED);
      $comments_per_page = variable_get('comment_default_per_page_' . $node->type, 50);

      $query = db_select('photos_comment', 'v')->extend('PagerDefault');
      $query->join('comment', 'c', 'c.cid = v.cid');
      $query->addField('v', 'cid');
      $query->condition('v.fid', $fid)
        ->addTag('node_access')
        ->addTag('comment_filter')
        ->addMetaData('node', $node)
        ->limit($comments_per_page);

      $count_query = db_select('photos_comment', 'v');
      $count_query->join('comment', 'c', 'c.cid = v.cid');
      $count_query->addExpression('COUNT(*)');
      $count_query
        ->condition('v.fid', $fid)
        ->addTag('node_access')
        ->addTag('comment_filter')
        ->addMetaData('node', $node);

      if (!user_access('administer comments')) {
        $query->condition('c.status', COMMENT_PUBLISHED);
        $count_query->condition('c.status', COMMENT_PUBLISHED);
      }

      if ($mode === COMMENT_MODE_FLAT) {
        $query->orderBy('c.cid', 'ASC');
      }
      else {
        $query->addExpression('SUBSTRING(c.thread, 1, (LENGTH(c.thread) - 1))', 'torder');
        $query->orderBy('torder', 'ASC');
      }
      $query->setCountQuery($count_query);
      $cids = $query->execute()->fetchCol();

      if (!empty($cids)) {
        $comments = comment_load_multiple($cids);
        comment_prepare_thread($comments);
        $build = comment_view_multiple($comments, $node);
        $build['pager']['#theme'] = 'pager';
        $output['comments'] = $build;
      }
    }

    if ($node->comment == 2) {
      if (user_access('post comments') && variable_get('photos_comment', 0) || user_access('administer comments') && variable_get('photos_comment', 0)) {
        $build = drupal_get_form("comment_node_{$node->type}_form", (object) array('nid' => $node->nid));
        $output['comment_form'] = $build;
      }
    }
  }

  if ($output) {
    if (module_exists('ajax_comments')) {
      // Add support for ajax comments on image page.
      $output['comments']['#prefix'] = '<div id="comment-wrapper-nid-' . $node->nid . '">';
      $output['comments']['#prefix'] .= '<div class="ajax-comment-wrapper"></div>';
      $output['comments']['#suffix'] = '</div>';
    }
    $output += array(
      '#theme' => 'comment_wrapper__node_' . $node->type,
      '#node' => $node,
      'comments' => array(),
      'comment_form' => array(),
    );
  }

  return $output;
}
