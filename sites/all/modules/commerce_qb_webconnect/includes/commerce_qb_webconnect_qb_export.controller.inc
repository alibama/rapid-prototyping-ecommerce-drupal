<?php
class CommerceQbWebconnectQbExport extends Entity {
  public function __construct($values = array()) {
    parent::__construct($values, 'commerce_qb_webconnect_qb_export');
  }
  protected function defaultLabel() {
    return $this->id;
  }

  protected function defaultUri() {
    return array('path' => 'admin/commerce/qb-exports/' . $this->id);
  }
}
class CommerceQbWebconnectQbExportEntityController extends EntityAPIController {
  function attachLoad(&$queried_exports, $revision_id = FALSE) {
    foreach ($queried_exports as &$export) {
      $export->data = drupal_json_decode($export->data);
    }
  }
  /**
   * Builds a structured array representing the entity's content.
   *
   * The content built for the entity will vary depending on the $view_mode
   * parameter.
   *
   * @param $entity
   *   An entity object.
   * @param $view_mode
   *   View mode, e.g. 'full'
   * @param $langcode
   *   (optional) A language code to use for rendering. Defaults to the global
   *   content language of the current request.
   * @return
   *   The renderable array.
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $qbxml = commerce_qb_webconnect_qbxml($entity);

    $content['webconnect_table'] = array(
      '#markup' => '<pre>' . htmlspecialchars($qbxml) . '</pre>',
    );

    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }
}
