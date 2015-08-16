<?php

/**
 * @file
 * Administrative actions
 */

/**
 * Admin settings form
 */
function dropfort_update_admin_form($form, &$form_state) {

  if (!dropfort_update_connection_test()) {
    $form['dropfort_registration_link'] = array(
      '#markup' => '<p class="dropfort-registration disconnected"><strong>' . _dropfort_update_site_registration_link() . '</strong></p>',
      '#suffix' => '<p><em>' . t("Use the link above to register your site with Dropfort.") . '</em></p>',
    );
  }
  else {
    $form['dropfort_registration_link'] = array(
      '#prefix' => '<p class="dropfort-registration connected"><strong>' . t('Site Registered') . '</strong></p>',
      '#markup' => '<p>' . t('Your site has been successfully registered to Dropfort.') . '</p>',
    );
  }

  $form['dropfort_update_show_moderation_notice'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show moderation notice'),
    '#description' => t('Show a notification on update report pages that the list of updates are being moderated.'),
    '#default_value' => variable_get('dropfort_update_show_moderation_notice', FALSE),
  );

  $form['advanced_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#access' => user_access('administer dropfort_update'),
  );

  $form['advanced_settings']['dropfort_update_base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Base URL'),
    '#description' => t('Base URL to your Dropfort instance. <em>Defaults to @url</em>', array('@url' => DROPFORT_UPDATE_BASE_URL)),
    '#size' => 40,
    '#maxlength' => 255,
    '#default_value' => variable_get('dropfort_update_base_url', DROPFORT_UPDATE_BASE_URL),
  );

  $form['advanced_settings']['dropfort_update_allow_unencrypted'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow unencrypted connections'),
    '#description' => t('Allow update data to be requested via plain HTTP connections. <em>Not recommended</em>'),
    '#default_value' => variable_get('dropfort_update_allow_unencrypted', FALSE),
  );

  $form['advanced_settings']['dropfort_update_timestamp_delta'] = array(
    '#type' => 'textfield',
    '#title' => t('Timestamp delta'),
    '#description' => t('The maximum delta between the Dropfort server time and your site server time (in seconds). This is an absolute value.'),
    '#size' => 26,
    '#maxlength' => 32,
    '#default_value' => DROPFORT_UPDATE_TIMESTAMP_DELTA,
    '#element_validate' => array('element_validate_integer_positive'),
  );

  $form['advanced_settings']['dropfort_update_cache_lifetime'] = array(
    '#type' => 'textfield',
    '#title' => t('Cache lifetime'),
    '#description' => t('Time in seconds to cache data from remote Dropfort site'),
    '#default_value' => variable_get('dropfort_update_cache_lifetime', 86400),
    '#element_validate' => array('element_validate_integer_positive'),
  );

  $form['advanced_settings']['dropfort_update_site_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Site Key'),
    '#disabled' => TRUE,
    '#size' => 70,
    '#default_value' => dropfort_update_get_private_key(),
    '#description' => t('Currently active site key generated by your Drupal site. This value may vary depending on your <code>$base_url</code> settings.'),
  );

  $keys = dropfort_update_get_site_keys();
  $table_variables = array(
    'header' => array(
      t('Site Key'),
      t('State'),
      t('Last Used'),
      t('Connection Test'),
    ),
    'rows' => array(),
    //'caption' => t('Registered site keys. If any are marked as inactive use the provided links to quickly add your site keys to Dropfort.'),
    'caption' => t('Generated site keys.'),
  );
  foreach ($keys as $key => $value) {
    // Disable for now until we get the required services active on dropfort
    //if (!$value->active) {
    //  $path = variable_get('dropfort_update_base_url', DROPFORT_UPDATE_BASE_URL) . '/site/token/' . check_plain(variable_get('dropfort_update_site_token'));
    //  $value->site_key = l($value->site_key, $path, array('external' => TRUE, 'query' => array('site_key' => $value->site_key)));
    //}
    $value->last_used = format_date($value->last_used);
    $value->active = TRUE == $value->active ? t('Active') : t('Inactive');
    $test_result = dropfort_update_connection_test(FALSE, array('site_key' => $value->site_key));

    if ($test_result) {
      $value->registration_status = t('Connected');
    }
    else {
      $value->registration_status = t('Failed to connect');
    }
    $table_variables['rows'][] = (array)$value;
  }

  $form['advanced_settings']['site_keys'] = array(
    '#title' => t('Registered site keys'),
    '#markup' => theme('table', $table_variables),
  );

  $form['advanced_settings']['dropfort_update_clear_all_site_keys'] = array(
    '#type' => 'checkbox',
    '#title' => t('Clear all site keys'),
    '#default_value' => -1,
    '#description' => t('Clear all detected site keys and regenerate a new list. This may require you to update your registration information on Dropfort.com'),
  );

  $form['advanced_settings']['dropfort_update_site_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Site Token'),
    '#description' => t('Randomly generated token to allow access to your site to Dropfort. You may regenerate this key at any time however you must update your site information on Dropfort to match.'),
    '#size' => 70,
    '#default_value' => variable_get('dropfort_update_site_token'),
    '#maxlength' => 255,
    '#disabled' => TRUE,
  );

  $form['advanced_settings']['dropfort_update_generate_new_site_token'] = array(
    '#type' => 'checkbox',
    '#title' => t('Generate New Site Token'),
    '#description' => t('When saving the configuration, generate a new site token. Useful when an migrating a site between environments or when the value is no longer secret. <em>N.B. This will clear your cached project update data.</em>'),
    '#default_value' => -1,
  );

  $form['#validate'][] = 'dropfort_update_admin_validate';
  $form['#submit'][] = 'dropfort_update_admin_submit';
  return system_settings_form($form);
}

/**
 * Validation callback for dropfort admin form
 */
function dropfort_update_admin_validate(&$form, &$form_state) {
  // We don't want to save the site key or site id so we'll remove it from the submitted data
  unset($form_state['values']['dropfort_update_site_token']);
  unset($form_state['values']['dropfort_update_site_key']);
}

/**
 * Submit callback for dropfort admin form
 */
function dropfort_update_admin_submit(&$form, &$form_state) {
  if (isset($form_state['values']['dropfort_update_generate_new_site_token']) and FALSE != $form_state['values']['dropfort_update_generate_new_site_token']) {
    _dropfort_update_generate_new_site_token();
    drupal_set_message(t('Dropfort site token been regenerated.'));
  }

  // Delete all the current site keys
  if (isset($form_state['values']['dropfort_update_clear_all_site_keys']) and FALSE != $form_state['values']['dropfort_update_clear_all_site_keys']) {
    dropfort_update_clear_all_site_keys();
    drupal_set_message(t('Existing site keys have been cleared. Default site key has been regenerated.'));
  }

  // Reset status update time
  variable_set('dropfort_update_last_status', 0);
}