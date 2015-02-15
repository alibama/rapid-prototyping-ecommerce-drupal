<?php
/**
 * @file
 * Handles share image page(s) and content.
 */

/**
 * Photos download (share) page.
 */
function photos_down_page($fid) {
  $op = array();
  $content = array();
  $query = db_select('file_managed', 'f');
  $query->join('photos_image', 'p', 'p.fid = f.fid');
  $query->join('users', 'u', 'u.uid = f.uid');
  $query->fields('f', array('uri', 'filemime', 'timestamp', 'filename'))
    ->fields('u', array('uid', 'name'))
    ->fields('p')
    ->condition('p.fid', $fid);
  $image = $query->execute()->fetchObject();
  $page_title = t('Sharing code and download image: @title', array('@title' => $image->filename));
  drupal_set_title($page_title);
  if (!module_exists('colorbox')) {
    $content['back'] = l(t('Back to the image page'), 'photos/image/' . $fid);
  }
  $content['info'] = t('Uploaded on !time by !name', array('!name' => $image->name, '!time' => format_date($image->timestamp, 'small')));
  if (arg(3) != 'exif' && arg(3) != 'vote') {
    $image = photos_get_info(0, $image);
    $size_info = photos_upload_info();
    $content['sizes'] = $size_info['size'];
    $content['link']['original'] = user_access('view original') ? l(t('Original'), 'photos/zoom/' . $fid . '/original') : NULL;
    $op['class'] = array('active');
  }
  if (isset($image->exif) && variable_get('photos_exif', 0) && ($exif = _photos_down_exif($image->uri, $fid))) {
    $content['link']['exif'] = l(t('Exif'), 'photos/zoom/' . $fid . '/exif');
  }
  if (variable_get('photos_vote', 0) && user_access('view vote list')) {
    $content['link']['vote'] = l(t('Vote list'), 'photos/zoom/' . $fid . '/vote');
  }
  $sizes = isset($content['sizes']) ? $content['sizes'] : array();
  foreach ($sizes as $size) {
    $content['link'][$size['style']] = l(t($size['name']), 'photos/zoom/' . $fid . '/' . $size['style']);
  }
  switch (arg(3)) {
    case 'thumb':
      if ($style_name = _photos_down_label(arg(4))) {
        $content['view'] = theme('photos_imagehtml', array('style_name' => $style_name, 'image' => $image));
        $content['sharing_url'] = _photos_l($image->uri, $image->thumb[$label]);
        $content['sharing_link'] = url('photos/image/' . $fid, array('absolute' => TRUE));
        $content['sharing_title'] = $image['filename'];
      }
      else {
        $content['view'] = t('Without this size image');
      }
    break;
    case 'original':
      $content['view'] = theme('photos_imagehtml', array('style_name' => 'original', 'image' => $image));
    break;
    case 'exif':
      $content['view'] = $exif ? $exif : t('There is no Exif information for this image.');
    break;
    case 'vote':
      if (!user_access('view vote list')) {
        drupal_access_denied();
        return;
      }
      $content['view'] = _photos_down_vote($fid);
    break;
    default:
      if ($content['sizes'][0]['style']) {
        $style_name = arg(3);
        if (empty($style_name)) {
          $style_name = variable_get('photos_display_view_imagesize', 'large');
        }
        $content['view'] = theme('photos_imagehtml', array('style_name' => $style_name, 'image' => $image));
        if ($style_name == 'original') {
          $content['sharing_url'] = _photos_l($image->uri, $style_name);
        }
        else {
          $content['sharing_url'] = _photos_l($image->uri, $style_name);
        }
      }
      $content['sharing_link'] = url('photos/image/' . $fid, array('absolute' => TRUE));
      $content['sharing_title'] = $image->filename;
  }
  $content['link']['sizes'] = l(t('All sizes'), 'photos/zoom/' . $fid);
  if (variable_get('photos_print_sizes', 1)) {
    print theme('photos_down', array('content' => $content, 'type' => 'print'));
  }
  else {
    return theme('photos_down', array('content' => $content, 'type' => 'return'));
  }
}

/**
 * Photos exif data.
 */
function _photos_down_exif($filepath, $fid, $clear = 0) {
  $cid = 'exif_' . $fid;
  if ($data = cache_get($cid, 'cache_photos')) {
    return theme('photos_exif', array('exif' => $data->data));
  }
  elseif ($exif = @exif_read_data($filepath, 0, TRUE)) {
    unset($data);
    $tags = _photos_exif_tag();
    foreach ($tags as $key => $tag) {
      if (!is_array($tag)) {
        if ($exif[$tag]) {
          $data[$tag] = $exif[$tag];
          print_r($exif[$tag]);
        }
      }
      else {
        foreach ($tag as $label) {
          if (isset($exif[$key][$label])) {
            $data[$key][$label] = $exif[$key][$label];
          }
        }
      }
    }
    if (isset($data)) {
      cache_set($cid, $data, 'cache_photos');
      return theme('photos_exif', array('exif' => $data));
    }
    else {
      db_query('UPDATE {photos_image} SET exif = 0 WHERE fid = :fid', array(':fid' => $fid));
    }
  }
}

/**
 * Download label.
 */
function _photos_down_label($label) {
  $t = photos_upload_info();
  foreach ($t['size'] as $v) {
    if ($v['name'] == $label) {
      return $label;
    }
  }
}

/**
 * Vote.
 */
function _photos_down_vote($fid) {
  $output = '';
  $header = array(
    array('data' => t('Vote user'), 'field' => 'v.uid'),
    array('data' => t('Vote result'), 'field' => 'v.value'),
    array('data' => t('Vote time'), 'field' => 'v.timestamp', 'sort' => 'desc'),
  );
  $rows = array();
  $query = db_select('votingapi_vote', 'v')->extend('PagerDefault')->extend('TableSort');
  $query->join('users', 'u', 'u.uid = v.uid');
  $query->fields('v', array('uid', 'value', 'timestamp', 'vote_source'))
    ->fields('u', array('name'))
    ->condition('v.entity_type', 'image')
    ->condition('v.entity_id', $fid)
    ->orderByHeader($header)
    ->limit(30);
  $result = $query->execute();
  foreach ($result as $a) {
    if ($a->uid != 0) {
      $name = theme('username', array('account' => $a));
    }
    else {
      $name = $a->vote_source;
    }
    $rows[] = array($name, $a->value, format_date($a->timestamp, 'small'));
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => t('No vote available.'), 'colspan' => 3));
  }
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= theme('pager');
  drupal_set_title(t('See a image vote'));

  return $output;
}

/**
 * Specify you want to save the exif field
 */
function _photos_exif_tag() {
  $tags = array(
    'IFD0' => array(
      'Make',
      'Model',
      'Orientation',
      'XResolution',
      'YResolution',
      'ResolutionUnit',
      'YCbCrPositioning',
      'Exif_IFD_Pointer',
      'GPS_IFD_Pointer',
    ),
    'EXIF' => array(
      'FNumber',
      'ExifVersion',
      'DateTimeOriginal',
      'DateTimeDigitized',
      'ComponentsConfiguration',
      'ApertureValue',
      'LightSource',
      'Flash',
      'FocalLength',
      'FlashPixVersion',
      'ColorSpace',
      'ExifImageWidth',
      'ExifImageLength',
      'CustomRendered',
      'ExposureMode',
      'WhiteBalance',
      'DigitalZoomRatio',
      'SceneCaptureType',
    ),
    'GPS' => array(
      'GPSVersion',
      'GPSLatitudeRef',
      'GPSLatitude',
      'GPSLongitudeRef',
      'GPSLongitude',
      'GPSAltitudeRef',
      'GPSAltitude',
      'GPSTimeStamp',
      'GPSSatellites',
      'GPSMeasureMode',
      'GPSDOP',
      'GPSSpeedRef',
      'GPSSpeed',
      'GPSTrackRef',
      'GPSTrack',
      'GPSMapDatum',
      'GPSDateStamp',
    ),
  );
  /*
  // Some possible exif tags.
  $tag = array(
    'FILE' => array(
      'FileName',
      'FileDateTime',
      'FileSize',
      'FileType',
      'MimeType',
      'SectionsFound',
    ),
    'COMPUTED' => array(
      'html',
      'Height',
      'Width',
      'IsColor',
      'ByteOrderMotorola',
      'ApertureFNumber',
      'Thumbnail.FileType',
      'Thumbnail.MimeType',
    ),
    'IFD0' => array(
      'Make',
      'Model',
      'Orientation',
      'XResolution',
      'YResolution',
      'ResolutionUnit',
      'YCbCrPositioning',
      'Exif_IFD_Pointer',
      'GPS_IFD_Pointer',
      'UndefinedTag:0xEA1C',
    ),
    'THUMBNAIL' => array(
      'Compression',
      'XResolution',
      'YResolution',
      'ResolutionUnit',
      'JPEGInterchangeFormat',
      'JPEGInterchangeFormatLength',
    ),
    'EXIF' => array(
      'FNumber',
      'ExifVersion',
      'DateTimeOriginal',
      'DateTimeDigitized',
      'ComponentsConfiguration',
      'ApertureValue',
      'LightSource',
      'Flash',
      'FocalLength',
      'FlashPixVersion',
      'ColorSpace',
      'ExifImageWidth',
      'ExifImageLength',
      'CustomRendered',
      'ExposureMode',
      'WhiteBalance',
      'DigitalZoomRatio',
      'SceneCaptureType',
      'UndefinedTag:0xEA1C',
    ),
    'GPS' => array(
      'GPSVersion',
      'GPSLatitudeRef',
      'GPSLatitude',
      'GPSLongitudeRef',
      'GPSLongitude',
      'GPSAltitudeRef',
      'GPSAltitude',
      'GPSTimeStamp',
      'GPSSatellites',
      'GPSMeasureMode',
      'GPSDOP',
      'GPSSpeedRef',
      'GPSSpeed',
      'GPSTrackRef',
      'GPSTrack',
      'GPSMapDatum',
      'GPSDateStamp',
    ),
    'WINXP' => array(
      'Title',
      'Comment',
      'Author',
      'Keywords',
      'Subject'
    ),
  );
  */
  return $tags;
}