#pragma once

#ifndef __STRINGOPERATOR__
#define __STRINGOPERATOR__

#include <Windows.h>
#include <iostream>
template <typename T>
class StringOperator
{
public:
    StringOperator(T msg)
        :m_str(msg)
    {
        m_nSizeStr = 0;

        m_charStr = new char[1];
        *m_charStr = '\0';
        m_wcharStr = new wchar_t[1];
        *m_wcharStr = '\0';
    }

    char* wchar2char();
    wchar_t* char2wchar();
    ~StringOperator()
    {
        delete[] m_charStr, m_wcharStr;
    }

private:
    T m_str;
    int m_nSizeStr;
    char* m_charStr;
    wchar_t* m_wcharStr;

};

template <typename T>
char* StringOperator<T>::wchar2char()
{
    m_nSizeStr = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)m_str, wcslen((const wchar_t*)m_str), NULL, 0, NULL, NULL);
    if (!m_nSizeStr)
    {
        return m_charStr;
    }
    delete m_charStr;
    m_charStr = new char[m_nSizeStr + 1];
    WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)m_str, wcslen((const wchar_t*)m_str), m_charStr, m_nSizeStr, NULL, NULL);
    m_charStr[m_nSizeStr] = '\0';
    return m_charStr;
}
template <typename T>
wchar_t* StringOperator<T>::char2wchar()
{
    m_nSizeStr = MultiByteToWideChar(CP_UTF8, 0, (char*)m_str, strlen((const char*)m_str), NULL, 0);
    if (!m_nSizeStr)
    {
        return m_wcharStr;
    }
    delete m_wcharStr;
    m_wcharStr = new wchar_t[m_nSizeStr + 1];
    MultiByteToWideChar(CP_UTF8, 0, (char*)m_str, strlen((const char*)m_str), m_wcharStr, m_nSizeStr);
    m_wcharStr[m_nSizeStr] = '\0';
    return m_wcharStr;
}

#endif // !__STRINGOPERATOR__