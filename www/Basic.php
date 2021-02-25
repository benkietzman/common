<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Basic
// -------------------
// begin                : 2014-01-09
// copyright            : kietzman.org
// email                : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//////////////////////////////////////////////////////////////////////////

/*! \file Basic.php
* \brief Basic Class
*
* Provides basic functionality.
*/

// {{{ includes
include_once('error_handler/ErrorHandler.php');
// }}}
//! Basic Class
/*!
* Provides basic functionality.
*/
class Basic
{
  // {{{ variables
  //! Stores the ErrorHandler class instantiation
  protected $m_errorHandler;
  // }}}
  // {{{ __construct()
  /*! \fn __construct()
  * \brief Instatiates the class.
  */
  public function __construct()
  {
    $this->m_errorHandler = new ErrorHandler();
    $this->m_errorHandler->enable();
  }
  // }}}
  // {{{ __destruct()
  /*! \fn __destruct()
  * \brief Does nothing.
  */
  public function __destruct()
  {
    $this->showErrors();
  }
  // }}}
  // {{{ errorHandler()
  /*! \fn errorHandler()
  * \brief Retrieves the error handler.
  * \return Returns the error handler.
  */
  public function errorHandler()
  {
    return $this->m_errorHandler;
  }
  // }}}
  // {{{ getParam()
  /*! \fn getParam($strParamName, $strDefault = null)
  * \brief Fetches a GET/POST variable.
  * \param $strParamName Contains the name of the GET/POST variable to be fetched.
  * \param $strDefault Contains the default return value if the GET/POST variable does not exist.
  * \return Returns the value of the GET/POST variable.
  */
  public function getParam($strParamName, $strDefault = null)
  {
    return (isset($_POST[$strParamName]))?$_POST[$strParamName]:((isset($_GET[$strParamName]))?$_GET[$strParamName]:$strDefault);
  }
  // }}}
  // {{{ paramExists()
  /*! \fn paramExists($strParamName)
  * \brief Determines if a GET/POST variable exists.
  * \param $strParamName Contains the name of the variable to be checked.
  * \return Returns a boolean true/false.
  */
  public function paramExists($strParamName)
  {
    return (isset($_POST[$strParamName]) || isset($_GET[$strParamName]))?true:false;
  }
  // }}}
  // {{{ showErrors()
  /*! \fn showErrors()
  * \brief Writes all errors to the HTML document.
  */
  public function showErrors()
  {
    if ($this->m_errorHandler->messagesExist())
    {
      echo '<br><br>';
      echo '<hr>';
      echo '<p>';
      echo '<h2 align="center">The Following Errors Occured</h2>';
      $this->m_errorHandler->showMessages();
      echo '</p>';
    }
  }
  // }}}
}
// {{{ replacePngTags()
/*! \fn replacePngTags($x, $img_path = null, $sizeMeth = 'scale', $inScript = false)
* \brief Modifies PNG image tags to display transparency in Microsoft Internet Explorer.
* \param $x Contains the entire HTML img tag.
* \param $img_path Contains an alternate image path.
* \param $sizeMeth Contains the method for sizing the image.
* \param $inScript Contains a boolean value for whether Javascript logic should be analyzed.
*/
function replacePngTags($x, $img_path = null, $sizeMeth = 'scale', $inScript = false)
{
  $arr2=array();
  $msie='/msie\s(5\.[5-9]|[6]\.[0-9]*).*(win)/i';
  if (!isset($_SERVER['HTTP_USER_AGENT']) || !preg_match($msie,$_SERVER['HTTP_USER_AGENT']) || preg_match('/opera/i',$_SERVER['HTTP_USER_AGENT']))
  {
    return $x;
  }
  if ($inScript)
  {
    $saved_scripts=array();
    $placeholders=array();
    preg_match_all('`<script[^>]*>(.*)</script>`isU',$x,$scripts);
    for($i=0;$i<count($scripts[0]);$i++)
    {
      $x=str_replace($scripts[0][$i],'replacePngTags_ScriptTag-'.$i,$x);
      $saved_scripts[]=$scripts[0][$i];
      $placeholders[]='replacePngTags_ScriptTag-'.$i;
    }
  }
  preg_match_all('/background-image:\s*url\(([\\"\\\']?)([^\)]+\.png)\1\);/Uis',$x,$background);
  for($i=0;$i<count($background[0]);$i++)
  {
    $x=str_replace($background[0][$i],'filter:progid:DXImageTransform.'.
    'Microsoft.AlphaImageLoader(enabled=true, sizingMethod='.$sizeMeth.
    ', src=\''.$background[2][$i].'\');',$x);
  }
  $pattern='/<(input|img)[^>]*src=([\\"\\\']?)([^>]*\.png)\2[^>]*>/i';
  preg_match_all($pattern,$x,$images);
  for($num_images=0;$num_images<count($images[0]);$num_images++)
  {
    $original=$images[0][$num_images];
    $quote=$images[2][$num_images];
    $atts=''; $width=0; $height=0; $modified=$original;
    if(empty($img_path))
    {
      $tmp=split('[\\/]',$images[3][$num_images]);
      $this_img=array_pop($tmp);
      $img_path=join('/',$tmp);
      if(empty($img_path))
      {
        $tmp=split('[\\/]',$_SERVER['SCRIPT_NAME']);
        array_pop($tmp);
        $img_path=join('/',$tmp).'/';
      }
      else
      {
        $img_path.='/';
      }
    }
    else if(substr($img_path,-1)!='/')
    {
      $img_path.='/';
    }
    preg_match_all('/style=([\\"\\\']).*(\s?width:\s?([0-9]+(px|%));).*(\s?height:\s?([0-9]+(px|%));).*\\1/Ui', $images[0][$num_images],$arr2);
    if(is_array($arr2) && count($arr2[0]))
    {
      $width=$arr2[3][0];
      $height=$arr2[6][0];
      $stripper=str_replace(' ','\s','/('.$arr2[2][0].'|'.$arr2[5][0].')/');
      $modified=preg_replace('`style='.$arr2[1][0].$arr2[1][0].'`i', '', preg_replace($stripper,'',$modified));
    }
    else
    {
      preg_match_all('/width=([\\"\\\']?)([0-9%]+)\\1/i',$images[0][$num_images],$arr2);
      if(is_array($arr2) && count($arr2[0]))
      {
        $width=$arr2[2][0];
        if(is_numeric($width))
        $width.='px';
        $modified=str_replace($arr2[0][0],'',$modified);
      }
      preg_match_all('/height=([\\"\\\']?)([0-9%]+)\\1/i',$images[0][$num_images],$arr2);
      if(is_array($arr2) && count($arr2[0]))
      {
        $height=$arr2[2][0];
        if(is_numeric($height))
        $height.='px';
        $modified=str_replace($arr2[0][0],'',$modified);
      }
    }
    if($width==0 || $height==0)
    {
      if(file_exists($_SERVER['DOCUMENT_ROOT'].$img_path.$images[3][$num_images]))
      {
        $size=getimagesize($_SERVER['DOCUMENT_ROOT'].$img_path.$images[3][$num_images]);
        $width=$size[0].'px';
        $height=$size[1].'px';
      }
      else if(file_exists($_SERVER['DOCUMENT_ROOT'].$images[3][$num_images]))
      {
        $size=getimagesize($_SERVER['DOCUMENT_ROOT'].$images[3][$num_images]);
        $width=$size[0].'px';
        $height=$size[1].'px';
      }
      else
      {
        $piece = explode('/', $_SERVER['SCRIPT_NAME']);
        $temppath = '';
        for ($i = 0; $i < sizeof($piece) - 1; $i++)
        {
          $temppath .= $piece[$i].'/';
        }
        $size=getimagesize('http://'.$_SERVER['SERVER_NAME'].(($images[3][$num_images][0]!='/')?$temppath:'').$images[3][$num_images]);
        $width=$size[0].'px';
        $height=$size[1].'px';
      }
    }
    $replace_src_with=$quote.$img_path.'spacer.png'.$quote.' style="width: '.$width.'; height: '.$height.'; filter: progid:DXImageTransform.'.'Microsoft.AlphaImageLoader(src=\''.$images[3][$num_images].'\', sizingMethod='.$sizeMeth.');"';
    $new_tag=str_replace($quote.$images[3][$num_images].$quote,$replace_src_with, str_replace('  ',' ',$modified));
    $x=str_replace($original,$new_tag,$x);
  }
  if($inScript)
  {
    $x=str_replace($placeholders,$saved_scripts,$x);
  }

  return $x;
}
// }}}
?>
