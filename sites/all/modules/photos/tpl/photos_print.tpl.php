<?php
/**
 * @file
 * Formats print page.
 */
?>
<!DOCTYPE HTML>
<html lang="<?php print $GLOBALS['language']->language; ?>" xml:lang="<?php print $GLOBALS['language']->language; ?>">
<head>
  <title><?php print drupal_get_title(); ?></title>
  <?php print drupal_get_html_head(); ?>
  <?php print drupal_get_css(); ?>
  <?php print drupal_get_js('header'); ?>
  <style>
  #photos_print{
    padding: 10px;
  }
  #photos_print .form-item{
    margin: 4px 0;
  }
  .photos_share_textarea {
    display:block;
  }
  .photos_share_textarea, .image-quote-link{
    width: 98%;
    margin: 6px 0;
  }
  </style>
</head>
<body>
<div id="photos_print">
  <?php if ($type == 'share'): ?>
    <?php
    if ($content['html']['image']) {
      $html = '<a href="' . $content['html']['link'] . '" title="' . $content['html']['title'] . '"><img src="' . $content['html']['image'] . '"/></a>';
     }
     else {
      $html = '<a href="' . $content['html']['link'] . '" title="' . $content['html']['title'] . '">' . $content['html']['title'] . '</a>';
    }
    ?>
    <h2>HTML</h2>
    <input type="text" value="<?php print check_plain($html); ?>" class="image-quote-link"/>
    <h2>EMBED</h2>
    <textarea class="photos_share_textarea" rows="6" cols="60"><?php print $content['flash']; ?></textarea>
  <?php else: ?>
    <?php print theme('status_messages'); ?>
    <?php print $content; ?>
  <?php endif; ?>
</div>
<?php print drupal_get_js('footer'); ?>
</body>
</html>
