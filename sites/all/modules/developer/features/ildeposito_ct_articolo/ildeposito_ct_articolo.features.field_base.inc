<?php
/**
 * @file
 * ildeposito_ct_articolo.features.field_base.inc
 */

/**
 * Implements hook_field_default_field_bases().
 */
function ildeposito_ct_articolo_field_default_field_bases() {
  $field_bases = array();

  // Exported field_base: 'field_prova_data'
  $field_bases['field_prova_data'] = array(
    'active' => 1,
    'cardinality' => -1,
    'deleted' => 0,
    'entity_types' => array(),
    'field_name' => 'field_prova_data',
    'foreign keys' => array(),
    'indexes' => array(),
    'locked' => 0,
    'module' => 'date',
    'settings' => array(
      'cache_count' => 4,
      'cache_enabled' => 0,
      'granularity' => array(
        'day' => 'day',
        'hour' => 0,
        'minute' => 0,
        'month' => 'month',
        'second' => 0,
        'year' => 'year',
      ),
      'repeat' => 1,
      'timezone_db' => '',
      'todate' => '',
      'tz_handling' => 'none',
    ),
    'translatable' => 0,
    'type' => 'datetime',
  );

  return $field_bases;
}
