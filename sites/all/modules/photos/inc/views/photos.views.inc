<?php

/**
 * @file
 * Integrate with the views module.
 */

/**
 * Implements hook_views_data().
 */
function photos_views_data() {
  // Group
  $data['photos_album']['table']['group'] = t('Album Photos');

  // Base albums table.
  $data['photos_album']['table']['base'] = array(
    'field' => 'pid',
    'title' => t('Photos Albums'),
    'help' => t('Albums.'),
    'weight' => -10,
  );

  // Join album info to images.
  $data['photos_album']['table']['join'] = array(
    'photos_image' => array(
      'left_field' => 'pid',
      'field' => 'fid',
    ),
  );
  // Node ID table field.
  $data['photos_album']['pid'] = array(
    'title' => t('Album ID'),
    'help' => t('This albums {node}.nid.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  // File ID table field.
  $data['photos_album']['fid'] = array(
    'title' => t('Album cover'),
    'help' => t('The {file_managed}.fid for the albums cover (if set).'),
    'field' => array(
      'handler' => 'photos_handler_field_photos_album_cover',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Has Image'),
      'type' => 'yes-no',
    ),
    // Define a relationship to the {file_managed} table.
    'relationship' => array(
      'base' => 'file_managed',
      'base field' => 'fid',
      'handler' => 'views_handler_relationship',
      'label' => t('Cover'),
      'title' => t('The cover associated with this album.'),
      'help' => t('Access to the file associated with the cover of this Album.'),
    ),
  );
  // Album weight.
  $data['photos_album']['wid'] = array(
    'title' => t('Album Weight'),
    'help' => t('The weight of this album.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  // Album image count.
  $data['photos_album']['count'] = array(
    'title' => t('Album Image Count'),
    'help' => t('The number of images in this album.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['photos_image']['table']['group'] = t('Album Photos');

  // Base images table.
  $data['photos_image']['table']['base'] = array(
    'field' => 'fid',
    'title' => t('Photos Images'),
    'help' => t('Album images.'),
    'weight' => -10,
  );

  // Join album info to images.
  $data['photos_image']['table']['join'] = array(
    'file_managed' => array(
      'left_field' => 'fid',
      'field' => 'fid',
    ),
  );

  // File ID table field.
  $data['photos_image']['fid'] = array(
    'title' => t('Image'),
    'help' => t('Album images.'),
    'field' => array(
      'handler' => 'photos_handler_field_photos_image',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Has Image'),
      'type' => 'yes-no',
    ),
    'relationship' => array(
      'base' => 'file_managed',
      'base field' => 'fid',
      'handler' => 'views_handler_relationship',
      'label' => t('Image file'),
      'title' => t('The file associated with this image.'),
      'help' => t('Access to the file information associated with this image.'),
    ),
  );
  // Node ID table field.
  $data['photos_image']['pid'] = array(
    'title' => t('Album ID'),
    'help' => t('This images album {node}.nid.'),
    // Define a relationship to the {node} table.
    'relationship' => array(
      'base' => 'node',
      'base field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Album: Node'),
      'title' => t('The node associated with this album.'),
      'help' => t('Access to node title and fields for this Album.'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  // Image title.
  $data['photos_image']['title'] = array(
    'title' => t('Image Title'),
    'help' => t('The title for this image.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );
  // Image description.
  $data['photos_image']['des'] = array(
    'title' => t('Image Description'),
    'help' => t('The description for this image.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => FALSE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Has Description'),
      'type' => 'yes-no',
    ),
  );
  // Image weight.
  $data['photos_image']['wid'] = array(
    'title' => t('Image Weight'),
    'help' => t('The image weight - custom sort order.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  // Image views count.
  $data['photos_image']['count'] = array(
    'title' => t('Image Views'),
    'help' => t('Count how many times this image has been viewed.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  // Image comment count.
  $data['photos_image']['comcount'] = array(
    'title' => t('Image Comments'),
    'help' => t('Count how many comments this image has.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  return $data;
}

/**
 * Implements hook_views_data_alter().
 */
function photos_views_data_alter(&$views_data) {
  // Join node info to albums.
  $views_data['node']['table']['join']['photos_album'] = array(
    'left_field' => 'pid',
    'field' => 'nid',
  );

  if (module_exists('votingapi')) {
    // Add relationship handlers for both votingapi tables, for album photos.
    $default_relationships[] = array(
      'description' => t('photos'),
      'entity_type' => 'image',
      'base_table' => 'photos_image',
      'entity_id_column' => 'fid',
      'pseudo_vote' => 'votingapi_vote',    // for legacy compatability w/RC1.
      'pseudo_cache' => 'votingapi_cache',  // for legacy compatability w/RC1.
    );

    foreach ($default_relationships as $data) {
      $pseudo = str_replace(array(' ', '-', '.'), '_', $data['entity_type'] . '_' . $data['entity_id_column']);
      $pseudo_vote = empty($data['pseudo_vote']) ? 'vapi_' . $pseudo : $data['pseudo_vote'];
      $pseudo_cache = empty($data['pseudo_cache']) ? 'vapic_' . $pseudo : $data['pseudo_cache'];

      $views_data[$data['base_table']][$pseudo_vote]['relationship'] = array(
        'title' => 'Votes',
        'help' => 'Votes cast by users on ' . $data['description'] . '.',
        'base' => 'votingapi_vote',
        'field' => 'entity_id',
        'relationship field' => $data['entity_id_column'],
        'handler' => 'votingapi_views_handler_relationship',
        'extra' => array(
          array(
            'field' => 'entity_type',
            'value' => $data['entity_type'],
            'numeric' => FALSE
          ),
        ),
      );

      $views_data[$data['base_table']][$pseudo_cache]['relationship'] = array(
        'title' => 'Vote results',
        'help' => 'Aggregate results of votes cast on ' . $data['description'] . '.',
        'base' => 'votingapi_cache',
        'field' => 'entity_id',
        'relationship field' => $data['entity_id_column'],
        'handler' => 'votingapi_views_handler_relationship',
        'extra' => array(
          array(
            'field' => 'entity_type',
            'value' => $data['entity_type'],
            'numeric' => FALSE
          ),
        ),
      );
    }
  }
}
 