<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Basic
// -------------------
// begin                : 2019-03-19
// copyright            : kietzman.org
// email                : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//////////////////////////////////////////////////////////////////////////

/*! \file statistic.php
* \brief Statistical Tracking
*
* Provides statistical tracking.
*/

include_once(dirname(__FILE__).'/database/Database.php');

$bProcessed = false;
$strError = null;
$post = json_decode(file_get_contents('php://input'), true);
if (is_array($post))
{
  if (isset($post['Application']))
  {
    if (is_array($_SERVER))
    {
      if (isset($_SERVER['HTTP_USER_AGENT']) && $_SERVER['HTTP_USER_AGENT'] != '')
      {
        if (isset($_SERVER['REMOTE_ADDR']) && $_SERVER['REMOTE_ADDR'] != '')
        {
          $bEnabled = true;
          if (file_exists(dirname(__FILE__).'/statistic.disable'))
          {
            $bEnabled = false;
          }
          if ($bEnabled)
          {
            $strConf = '/etc/central.conf';
            if (file_exists($strConf))
            {
              if (($handle = fopen($strConf, 'r')) !== false)
              {
                $strJson = null;
                while (!feof($handle))
                {
                  $strJson .= fgets($handle);
                }
                $json = json_decode($strJson, true);
                if (is_array($json))
                {
                  if (isset($json['Database User']) && $json['Database User'] != '')
                  {
                    if (isset($json['Database Password']) && $json['Database Password'] != '')
                    {
                      if (isset($json['Database']) && $json['Database'] != '')
                      {
                        if (isset($json['Database Server']) && $json['Database Server'] != '')
                        {
                          $database = new Database('MySql');
                          $db = $database->connect($json['Database User'], $json['Database Password'], $json['Database Server']);
                          $db->selectDatabase($json['Database']);
                          $getApplication = $db->parse('select id from application where name = \''.addslashes($post['Application']).'\'');
                          if ($getApplication->execute())
                          {
                            if (($getApplicationRow = $getApplication->fetch('assoc')))
                            {
                              $type = array('browser'=>0, 'operating_system'=>0, 'robot'=>0);
                              foreach ($type as $key => $value)
                              {
                                $getType = $db->parse('select * from '.$key);
                                while (($getTypeRow = $getType->fetch()))
                                {
                                  if (strstr($_SERVER['HTTP_USER_AGENT'], $getTypeRow['search']) !== false)
                                  {
                                    $type[$key] = $getTypeRow['id'];
                                  }
                                }
                                $db->free($getType);
                              }
                              $getStatistic = $db->parse('select `count` from statistic where application_id = '.$getApplicationRow['id'].' and browser_id = '.$type['browser'].' and operating_system_id = '.$type['operating_system'].' and robot_id = '.$type['robot'].' and ip_address = \''.$_SERVER['REMOTE_ADDR'].'\' and date_format(`date`, \'%Y-%m-%d\') = date_format(now(), \'%Y-%m-%d\')');
                              if (($getStatisticRow = $getStatistic->fetch()))
                              {
                                $updateStatistic = $db->parse('update statistic set `count` = '.($getStatisticRow['count'] + 1).' where application_id = '.$getApplicationRow['id'].' and browser_id = '.$type['browser'].' and operating_system_id = '.$type['operating_system'].' and robot_id = '.$type['robot'].' and ip_address = \''.$_SERVER['REMOTE_ADDR'].'\' and date_format(`date`, \'%Y-%m-%d\') = date_format(now(), \'%Y-%m-%d\')');
                                if ($updateStatistic->execute())
                                {
                                  $bProcessed = true;
                                }
                                else
                                {
                                  $strError = $updateStatistic->getError();
                                }
                              }
                              else
                              {
                                $insertStatistic = $db->parse('insert into statistic (application_id, browser_id, operating_system_id, robot_id, ip_address, `date`, `count`) values ('.$getApplicationRow['id'].', '.$type['browser'].', '.$type['operating_system'].', '.$type['robot'].', \''.$_SERVER['REMOTE_ADDR'].'\', now(), 1)');
                                if ($insertStatistic->execute())
                                {
                                  $bProcessed = true;
                                }
                                else
                                {
                                  $strError = $insertStatistic->getError();
                                }
                              }
                              $db->free($getStatistic);
                            }
                            else
                            {
                              $strError = 'Failed to query application from application table of central database.';
                            }
                          }
                          else
                          {
                            $strError = $getApplication->getError();
                          }
                          $db->free($getApplication);
                          $database->disconnect($db);
                        }
                        else
                        {
                          $strError = 'Failed find Database Server within Central JSON configuration.';
                        }
                      }
                      else
                      {
                        $strError = 'Failed find Database within Central JSON configuration.';
                      }
                    }
                    else
                    {
                      $strError = 'Failed find Database Password within Central JSON configuration.';
                    }
                  }
                  else
                  {
                    $strError = 'Failed find Database User within Central JSON configuration.';
                  }
                }
                else
                {
                  $strError = 'Failed to decode Central JSON configuration.';
                }
                unset($json);
                fclose($handle);
              }
              else
              {
                $strError = 'Failed to open '.$strConf.' for reading.';
              }
            }
            else
            {
              $strError = 'Failed to find '.$strConf.'.';
            }
          }
          else
          {
            $bProcessed = true;
            $strError = 'Statistics are disabled.';
          }
        }
        else
        {
          $strError = 'Failed to find REMOTE_ADDR within SERVER data.';
        }
      }
      else
      {
        $strError = 'Failed to find HTTP_USER_AGENT within SERVER data.';
      }
    }
    else
    {
      $strError = 'Failed to find SERVER data.';
    }
  }
  else
  {
    $strError = 'Failed to find Application within JSON input.';
  }
}
else
{
  $strError = 'Failed to decode POST JSON input.';
}
unset($post);
$response = array();
$response['Status'] = (($bProcessed)?'okay':'error');
if ($strError != '')
{
  $response['Error'] = $strError;
}
echo json_encode($response)."\n";
?>
