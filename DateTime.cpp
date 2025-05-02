/* -*- c++ -*- */
///////////////////////////////////////////
// DateTime
// -------------------------------------
// file       : DateTime.cpp
// author     : Ben Kietzman
// begin      : 2010-03-03
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file DateTime.cpp
* \brief DateTime Class
*/
// {{{ includes
#include "DateTime"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ DateTime()
    DateTime::DateTime()
    {
    }
    // }}}
    // {{{ ~DateTime()
    DateTime::~DateTime()
    {
    }
    // }}}
    // {{{ civilianToMilitary()
    void DateTime::civilianToMilitary(int &nHour, const string strTimeOfDay)
    {
      if (strTimeOfDay == "AM")
      {
        if (nHour == 12)
        {
          nHour = 0;
        }
      }
      else
      {
        nHour += 12;
      }
    }
    // }}}
    // {{{ militaryToCivilian()
    void DateTime::militaryToCivilian(int &nHour, string &strTimeOfDay)
    {
      if (nHour <= 12)
      {
        strTimeOfDay = "AM";
        if (nHour == 0)
        {
          nHour = 12;
        }
      }
      else
      {
        strTimeOfDay = "PM";
        if (nHour > 12)
        {
          nHour -= 12;
        }
      }
    }
    // }}}
    // {{{ isLeapYear()
    bool DateTime::isLeapYear(const int nYear)
    {
      bool bResult = false;

      if ((!(nYear % 4) && (nYear % 100)) || !(nYear % 400))
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ addYears()
    void DateTime::addYears(int &nYear, const int nNumber)
    {
      nYear += nNumber;
    }
    // }}}
    // {{{ addMonths()
    void DateTime::addMonths(int &nYear, int &nMonth, const int nNumber)
    {
      int nExtra = nNumber / 12;

      nMonth += nNumber % 12;
      if (nNumber > 0)
      {
        if (nMonth > 12)
        {
          nExtra++;
          nMonth -= 12;
        }
      }
      else if (nNumber < 0)
      {
        if (nMonth < 1)
        {
          nExtra--;
          nMonth += 12;
        }
      }
      addYears(nYear, nExtra);
    }
    // }}}
    // {{{ addDays()
    void DateTime::addDays(int &nYear, int &nMonth, int &nDay, const int nNumber)
    {
      int nTempNumber = nNumber, nDaysInMonth = 0;

      if (nNumber > 0)
      {
        daysInMonth(nYear, nMonth, nDaysInMonth);
        while ((nDay + nTempNumber) > nDaysInMonth)
        {
          nTempNumber -= (nDaysInMonth - nDay) + 1;
          addMonths(nYear, nMonth, 1);
          daysInMonth(nYear, nMonth, nDaysInMonth);
          nDay = 1;
        }
      }
      else if (nNumber < 0)
      {
        while ((nDay + nTempNumber) < 1)
        {
          nTempNumber += nDay;
          addMonths(nYear, nMonth, -1);
          daysInMonth(nYear, nMonth, nDay);
        }
      }
      nDay += nTempNumber;
    }
    // }}}
    // {{{ addHoursCivilian()
    void DateTime::addHoursCivilian(int &nYear, int &nMonth, int &nDay, int &nHour, string &strTimeOfDay, const int nNumber)
    {
      civilianToMilitary(nHour, strTimeOfDay);
      addHoursMilitary(nYear, nMonth, nDay, nHour, nNumber);
      militaryToCivilian(nHour, strTimeOfDay);
    }
    // }}}
    // {{{ addHoursMilitary()
    void DateTime::addHoursMilitary(int &nYear, int &nMonth, int &nDay, int &nHour, const int nNumber)
    {
      int nExtra = nNumber / 24;

      nHour += nNumber % 24;
      if (nNumber > 0)
      {
        if (nHour > 24)
        {
          nExtra++;
          nHour -= 24;
        }
      }
      else if (nNumber < 0)
      {
        if (nHour < 1)
        {
          nExtra--;
          nHour += 24;
        }
      }
      addDays(nYear, nMonth, nDay, nExtra);
    }
    // }}}
    // {{{ addMinutesCivilian()
    void DateTime::addMinutesCivilian(int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, string &strTimeOfDay, const int nNumber)
    {
      civilianToMilitary(nHour, strTimeOfDay);
      addMinutesMilitary(nYear, nMonth, nDay, nHour, nMinute, nNumber);
      militaryToCivilian(nHour, strTimeOfDay);
    }
    // }}}
    // {{{ addMinutesMilitary()
    void DateTime::addMinutesMilitary(int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, const int nNumber)
    {
      int nExtra = nNumber / 60;

      nMinute += nNumber % 60;
      if (nNumber > 0)
      {
        if (nMinute > 59)
        {
          nExtra++;
          nMinute -= 60;
        }
      }
      else if (nNumber < 0)
      {
        if (nMinute < 0)
        {
          nExtra--;
          nMinute += 60;
        }
      }
      addHoursMilitary(nYear, nMonth, nDay, nHour, nExtra);
    }
    // }}}
    // {{{ addSecondsCivilian()
    void DateTime::addSecondsCivilian(int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, string &strTimeOfDay, const int nNumber)
    {
      civilianToMilitary(nHour, strTimeOfDay);
      addSecondsMilitary(nYear, nMonth, nDay, nHour, nMinute, nSecond, nNumber);
      militaryToCivilian(nHour, strTimeOfDay);
    }
    // }}}
    // {{{ addSecondsMilitary()
    void DateTime::addSecondsMilitary(int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, const int nNumber)
    {
      int nExtra = nNumber / 60;

      nSecond += nNumber % 60;
      if (nNumber > 0)
      {
        if (nSecond > 59)
        {
          nExtra++;
          nSecond -= 60;
        }
      }
      else if (nNumber < 0)
      {
        if (nSecond < 0)
        {
          nExtra--;
          nSecond += 60;
        }
      }
      addMinutesMilitary(nYear, nMonth, nDay, nHour, nMinute, nExtra);
    }
    // }}}
    // {{{ daysInYear()
    int DateTime::daysInYear(int &nDaysInYear)
    {
      int nYear;

      return daysInYear(getYear(nYear), nDaysInYear);
    }
    int DateTime::daysInYear(const int nYear, int &nDaysInYear)
    {
      if (isLeapYear(nYear))
      {
        nDaysInYear = 366;
      }
      else
      {
        nDaysInYear = 365;
      }

      return nDaysInYear;
    }
    // }}}
    // {{{ daysInMonth()
    int DateTime::daysInMonth(int &nDaysInMonth)
    {
      int nYear, nMonth;

      return daysInMonth(getYear(nYear), getMonth(nMonth), nDaysInMonth);
    }
    int DateTime::daysInMonth(const int nMonth, int &nDaysInMonth)
    {
      int nYear;

      return daysInMonth(getYear(nYear), nMonth, nDaysInMonth);
    }
    int DateTime::daysInMonth(const int nYear, const int nMonth, int &nDaysInMonth)
    {
      if (nMonth == 2)
      {
        if (isLeapYear(nYear))
        {
          nDaysInMonth = 29;
        }
        else
        {
          nDaysInMonth = 28;
        }
      }
      else if (nMonth == 4 || nMonth == 6 || nMonth == 9 || nMonth == 11)
      {
        nDaysInMonth = 30;
      }
      else
      {
        nDaysInMonth = 31;
      }

      return nDaysInMonth;
    }
    // }}}
  }
}
