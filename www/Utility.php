<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Utility
// -------------------
// begin                : 2022-01-18
// copyright            : kietzman.org
// email                : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//////////////////////////////////////////////////////////////////////////

/*! \file Utility.php
* \brief Utility Class
*
* Provides utility tools.
*/
class Utility
{
  // {{{ createClientStream()
  public function createClientStream($strServer, $port, $streamContext, &$strError)
  {
    $fdStream = stream_socket_client('tls://'.$strServer.':'.$port, $errno, $errstr, 10, STREAM_CLIENT_CONNECT, $streamContext);
    if ($fdStream === false)
    {
      $strError = 'createClientStream()->stream_socket_client('.$errno.') '.$errstr;
    }

    return $fdStream;
  }
  // }}}
  // {{{ createServerStream()
  public function createServerStream($strServer, $port, $streamContext, &$strError)
  {
    $fdStream = stream_socket_server('tls://'.$strServer.':'.$port, $errno, $errstr, STREAM_SERVER_BIND|STREAM_SERVER_LISTEN, $streamContext);
    if ($fdStream === false)
    {
      $strError = 'createServerStream()->stream_socket_server('.$errno.') '.$errstr;
    }

    return $fdStream;
  }
  // }}}
  // {{{ initClientStreamContext()
  public function initClientStreamContext($certificate, $privateKey, &$strError)
  {
    if ($clientStreamContext = stream_context_create())
    {
      stream_context_set_option($clientStreamContext, 'ssl', 'local_cert', $certificate);
      stream_context_set_option($clientStreamContext, 'ssl', 'local_pk',   $privateKey);
      stream_context_set_option($clientStreamContext, 'ssl', 'crypto_type', STREAM_CRYPTO_METHOD_TLS_CLIENT);
      stream_context_set_option($clientStreamContext, 'ssl', 'allow_self_signed', true);
      stream_context_set_option($clientStreamContext, 'ssl', 'verify_peer', false);
      stream_context_set_option($clientStreamContext, 'ssl', 'verify_peer_name', false);
    }
    else
    {
      $strError = 'initclientStreamContext()->stream_context_create() ' . var_dump(error_get_last()); 
    }

    return $clientStreamContext;
  }
  // }}}
  // {{{ initServerStreamContext()
  public function initServerStreamContext($certificate, $privateKey, &$strError)
  {
    if ($serverStreamContext = stream_context_create())
    {
      stream_context_set_option($serverStreamContext, 'ssl', 'local_cert', $certificate);
      stream_context_set_option($serverStreamContext, 'ssl', 'local_pk',   $privateKey);
      stream_context_set_option($serverStreamContext, 'ssl', 'crypto_type', STREAM_CRYPTO_METHOD_TLS_SERVER);
      stream_context_set_option($serverStreamContext, 'ssl', 'allow_self_signed', true);
      stream_context_set_option($serverStreamContext, 'ssl', 'verify_peer', false);
      stream_context_set_option($serverStreamContext, 'ssl', 'verify_peer_name', false);
    }
    else
    {
      $strError = 'initServerStreamContext()->stream_context_create() ' . var_dump(error_get_last()); 
    }

    return $serverStreamContext;
  }
  // }}}
}
?>
