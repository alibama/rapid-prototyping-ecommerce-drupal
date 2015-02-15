<?php
/**
 * @file
 * Formats swfu form.
 */
// @todo rename upload button to something like "select images".
$pid = '';
if (is_numeric(arg(1))) {
  $pid = arg(1);
}
elseif (isset($_GET['pid']) && is_numeric($_GET['pid'])) {
  $pid = $_GET['pid'];
}
?>
<script type="text/javascript">
  <?php if (!empty($pid)): ?>
  var swfu;
  window.onload = function() {
    var settings = {
      flash_url : "<?php if (isset($v['swf'])) echo $v['swf']; ?>",
      upload_url: "<?php echo $v['url']; ?>",
      // @todo pass token instead of phpsessid.
      post_params: {"PHPSESSID" : "<?php echo session_id(); ?>"},
      file_size_limit : "<?php echo $v['max_file_size']; ?> MB",
      file_types : "*.jpg;*.png;*.gif;*.jpeg",
      file_types_description : Drupal.t("All images..."),
      file_upload_limit : "<?php echo $v['num_uploads']; ?>",
      file_queue_limit : 0,
      custom_settings : {
        progressTarget : "album-swfu",
        cancelButtonId : "btnCancel"
      },
      auto_upload: false,
      debug: false,
      button_width: "70",
      button_height: "22",
      button_placeholder_id: "SWFUpload_0BrowseBtn",
      button_image_url : "<?php echo $v['image']; ?>",
      file_queued_handler : fileQueued,
      file_queue_error_handler : fileQueueError,
      file_dialog_complete_handler : fileDialogComplete,
      upload_start_handler : uploadStart,
      upload_progress_handler : uploadProgress,
      upload_error_handler : uploadError,
      upload_success_handler : uploadSuccess,
      upload_complete_handler : uploadComplete,
      queue_complete_handler : queueComplete
    };
    swfu = new SWFUpload(settings);
  };
  <?php endif; ?>

  (function ($) {
    $("select[name='pid']").change( function() {
      t = "<?php echo $v['href']; ?>" + '&pid=' + $("select[name='pid'] option:selected").val();
      window.location.href = t;
    });
  })(jQuery);
</script>

<div id="photos-swfu">
  <h4 id="queueinfo"><?php print t('Queue is empty'); ?></h4>
  <?php if (isset($v['link'])): ?>
    <h2 class=""><?php print $v['link']; ?></h2>
  <?php endif; ?>
  <span id="SWFUpload_0BrowseBtn" class="swfuploadbtn browsebtn photos_swfu_button" style="color:#F00;">
    <?php print t('Loading...'); ?>
  </span>
  <input id="btnCancel" type="button" value="<?php print t('Cancel All Uploads'); ?>" onclick="swfu.cancelQueue();" disabled="disabled"/>
  <input id="startUpload" class="photos_swfu_button" type="button" onclick="swfu.startUpload()" value="<?php print t('Click here to start upload'); ?>" disabled="disabled"/>
  <div id="album-swfu"></div>
  <?php if (empty($pid)): ?>
    <p><?php print t('Please select album to upload photos.'); ?></p>
  <?php endif; ?>
</div>