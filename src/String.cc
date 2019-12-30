// $RCSfile: String.cc,v $
// $Revision: 1.5 $

#include <stdio.h>  // needed for file functions:   *f()   etc.
#include <string.h> // needed for mpChar functions: str*() etc.

#include "String.h"
//
#include "Global.h"

M_ExternConstructDestructCountInt;


String::String() :
  mpChar(0),
  mSize(eStringInitSize)
{
  if (mSize) {
    mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);
    mpChar[0] = '\0';
  }

  M_UpdConstructCount(eType_String);
}

String::String(char* str) :
  mpChar(0),
  mSize(eStringInitSize)
{
  if (str) {
    int l = strlen(str);

    if (l >= mSize) mSize = l+1;

    mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);
    strcpy(mpChar, str);
  }
  else {
    if (mSize) {
      mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);
      mpChar[0] = '\0';
    }
  }

  M_UpdConstructCount(eType_String);
}

String::String(const char* str) :
// Same as char* constructor
  mpChar(0),
  mSize(eStringInitSize)
{
  if (str) {
    int l = strlen(str);

    if (l >= mSize) mSize = l+1;

    mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);
    strcpy(mpChar, str);
  }
  else {
    if (mSize) {
      mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);
      mpChar[0] = '\0';
    }
  }

  M_UpdConstructCount(eType_String);
}

String::String(const String& str) :
  mpChar(0),
  mSize(eStringInitSize)
{
  int l = str.Length();

  if (l >= mSize) mSize = l+1;

  mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);

  if (str.mpChar) strcpy(mpChar, str.mpChar);

  M_UpdConstructCount(eType_String);
}

String::String(String& str, short startpos) :
  mpChar(0),
  mSize(eStringInitSize)
{
  int l = str.Length();

  if (l > startpos) {
    if ((l-startpos) > mSize) mSize = l-startpos+1;
  }

  if (mSize) mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);

  if (l > startpos) strcpy(mpChar, &str.mpChar[l-startpos]);

  M_UpdConstructCount(eType_String);
}

String::String(String& str, short startpos, short endpos) :
  mpChar(0),
  mSize(eStringInitSize)
{
  int l = str.Length();

  if (endpos > l) endpos = l; // endpos not valid, truncate

  if ((endpos - startpos + 1) > mSize) mSize = endpos - startpos + 2;

  char* ptr    = str.mpChar + startpos;
  char* endptr = str.mpChar + endpos;

  if (startpos <= endpos) {
    for (int i=0; i<=(endpos-startpos); i++) {
      mpChar[i] = str.mpChar[startpos + i];
    }
  }

  mpChar[endpos-startpos+1] = '\0';

  M_UpdConstructCount(eType_String);
}

String::String(String& str, char* startstr) :
  mpChar(0),
  mSize(eStringInitSize)
{
  int pos = str.Position(startstr);

  int l;

  if (pos != -1) l = strlen(&str.mpChar[pos]);
  else           l = 0;

  if (l > mSize) mSize = l;

  if (mSize) {
    mpChar = new char [mSize]; M_UpdConstructCountN(eType_Char, mSize);

    if (pos != -1) strcpy(mpChar, &str.mpChar[pos]);
    else           mpChar[0] = '\0';
  }

  M_UpdConstructCount(eType_String);
}

String::String(String& str, String& startstr) :
  mpChar(0),
  mSize(eStringInitSize)
{
  int pos = str.Position(startstr);

  int l;

  if (pos != -1) l = strlen(&str.mpChar[pos]);
  else           l = 0;

  if (l > mSize) mSize = l;

  if (mSize) {
    mpChar = new char [mSize]; M_UpdConstructCountN(eType_Char, mSize);

    if (pos != -1) strcpy(mpChar, &str.mpChar[pos]);
    else           mpChar[0] = '\0';
  }

  M_UpdConstructCount(eType_String);
}

String::~String()
{
  if (mpChar != 0)
  {
    delete [] mpChar; M_UpdDestructCountN(eType_Char, mSize);
    mpChar = 0;
    mSize  = 0;
  }

  M_UpdDestructCount(eType_String);
}

int String::Length(void) const
{
  if (mpChar) return strlen(mpChar);
  else        return 0;
}

void String::Clear(void)
{
  if (mpChar) mpChar[0] = '\0';
}

void String::Append(const String& str)
{
  Append(str.mpChar);
}

void String::Append(char c)
{
  char buf[2];

  buf[0] = c, buf[1] = '\0';
  Append(buf);
}

void String::Append(const char* str)
{
  int l;

  if (str==NULL || (l=strlen(str))==0) return;

  int rem = mSize - Length(); // rem'ainder also needs to fit '\0'

  if (l >= rem)
  {
    String s(*this);

    // Check if we are appending to the same mpChar
    if (mpChar == str)
    {
      if (mpChar) { delete [] mpChar; mpChar = 0; M_UpdDestructCountN(eType_Char, mSize); }
      mSize = 2*l + 1;
      mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);
      strcpy(mpChar, s);
      strcat(mpChar, s);
    }
    else
    {
      if (mpChar) { delete [] mpChar; mpChar = 0; M_UpdDestructCountN(eType_Char, mSize); }
      mSize += l + 1;
      mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);
      strcpy(mpChar, s);
      strcat(mpChar, str);
    }
  }
  else
  {
    // Check if we are appending to the same mpChar
    if (mpChar == str)
    {
      String s(str);
      strcat(mpChar, (const char*)s);
    }
    else
      strcat(mpChar, str);
  }
}

void String::Append(char* str)
{
  Append((const char*) str);
}

void String::Prepend(char* str)
{
  String s(str);

  s.Append(*this);
  *this = s;
}

// Returns the number of substitutions made
short String::Replace(char cc, char dd)
{
  short count=0;
  char* r;
  int l1 = Length();

  const char* s = mpChar;
  while ((r=strchr(mpChar, (int)cc)) && l1)
  {
    *r = dd;
    count++;
    s = r+1;
  }

  return count;
}

// Returns the number of substitutions made
short String::Replace(char* str1, const char* str2)
{
  if (mpChar == NULL || str1 == NULL || strlen(str1) == 0)
    return 0;

  int count=0;
  String newstr;
  const char* repl = str2;

  if (str2 == NULL) repl = "";

  int l1 = strlen(str1);

  const char* s = mpChar;
  char* r;

  while (r=strstr(mpChar, str1))
  {
    r = '\0';
    newstr.Append(s);
    newstr.Append(repl);
    s += strlen(s) + l1;

    count++;
  }
  newstr.Append(s);

  *this = newstr;

  return count;
}

// TBD: Review functions below. Done reviewing above.

short String::Replace(char* str1, const char* str2, int n)
{
  if (mpChar == NULL || str1 == NULL || strlen(str1) == 0 || n == 0)
    return 0;

  int count=0;
  String newstr;
  const char* repl = str2;

  if (str2 == NULL) repl = "";

  char* r;
  int l1 = strlen(str1);

  const char* s = mpChar;
  while (r=strstr(mpChar, str1))
  {
    char save = *r;
    *r = '\0';
    newstr.Append(s);
    newstr.Append(repl);
    s += strlen(s) + l1;
    *r = save;

    count++;
    if (count == n)
      break;
  }
  newstr.Append(s);

  *this = newstr;

  return count;
}

short String::Occurences(char c) const
{
  short count=0;
  char* s = mpChar;
  char* r;

  while (r=strchr(s, c))
  {
    count++;
    s = r+1;
  }

  return count;
}

short String::Occurences(char* str) const
{
  char *s, *t;
  short count=0;
  int l = strlen(str);
  int m = Length();

  s = mpChar;
  while(l && m)
  {
    if (t=strstr(s, str))
    {
      count++;
      s = t+l;
    }
    else
      break;
  }

  return count;
}

short String::Occurences(const char* str) const
{
  return Occurences((char*)str);
}

int String::Position(char c) const
{
  char* r;

  r=strchr(mpChar, c);

  if (r)
    return (int)(r-mpChar);

  return -1;
}

int String::Position(char c, int n) const
{
  char *r, *t;
  int found=0;

  r = t = mpChar;
  while (found < n && r != 0)
  {
    r=strchr(t, c);
    if (r)
    {
      found++;
      t = r+1;
    }
    else
      break;
  }

  if (found)
    return (int)(t-mpChar-1);

  return -1;
}

int String::Position(const char* str) const
{
  char *s;

  if (s=strstr(mpChar, str))
    return (int)(s-mpChar);

  return -1;
}

int String::Position(const char* str, int n) const
{
  char* t;
  char* s = mpChar;
  for (int i=0; i<n; i++)
  {
    if (t=strstr(s, str))
    {
      s = t+1;
    }
    else
      return -1;
  }

  return (int)(t-mpChar);
}

int String::Position(int pos, char c) const
{
  if (pos>=Length() || pos<0)
    return -1;

  char* r;
  char* s = &mpChar[pos];

  r=strchr(s, c);

  if (r)
    return (int)(r-mpChar);

  return -1;
}

int String::Position(int pos, char c, int n) const
{
  if (pos>=Length() || pos<0)
    return -1;

  char *r, *t;
  int found=0;

  r = mpChar;
  t = &mpChar[pos];
  while (found < n && r != 0)
  {
    r=strchr(t, c);
    if (r)
    {
      found++;
      t = r+1;
    }
    else
      break;
  }

  if (found)
    return (int)(t-mpChar-1);

  return -1;
}

int String::Position(int pos, const char* str) const
{
  if (pos>=Length() || pos<0)
    return -1;

  char *s, *t;

  t = &mpChar[pos];

  if (s=strstr(t, str))
    return (int)(s-mpChar);

  return -1;
}

int String::Position(int pos, const char* str, int n) const
{
  if (pos>=Length() || pos<0)
    return -1;

  char* t;
  char* s = &mpChar[pos];

  for (int i=0; i<n; i++)
  {
    if (t=strstr(s, str))
    {
      s = t+1;
    }
    else
      return -1;
  }

  return (int)(t-mpChar);
}

void String::Insert(char* str, int pos)
{
  if (!(strlen(str)))
    return;

  if (pos > Length())
    return;

  char save = mpChar[pos];
  int rem = Length() - pos;
  if (rem)
  {
    String s(&mpChar[pos]);
    mpChar[pos] = '\0';
    Append(str);
    Append(s);
  }
  else
    Append(str);
}

void String::Insert(char c, int pos)
{
  Insert(c, pos, 1);
}

void String::Insert(char c, int pos, int count)
{
  char str[2];
  str[0] = c;
  str[1] = '\0';

  for (int i=0; i<count; i++)
    Insert(str, pos);
}

//String::operator char*() const
//{
//  return mpChar;
//}

String::operator const char*() const
{
  return mpChar;
}

String& String::operator<<(String &str)
{
  Append(str);

  return *this;
}

String& String::operator<<(char c)
{
  char buf[2];

  buf[0] = c, buf[1] = '\0';
  Append(buf);

  return *this;
}

String& String::operator<<(char* str)
{
  Append(str);

  return *this;
}

String& String::operator<<(const char* str)
{
  Append(str);

  return *this;
}

String& String::operator<<(short i)
{
  char buf[12];
  sprintf(buf, "%hd", i);
  Append(buf);

  return *this;
}

String& String::operator<<(int i)
{
  char buf[12];
  sprintf(buf, "%d", i);
  Append(buf);

  return *this;
}

String& String::operator<<(unsigned i)
{
  char buf[12];
  sprintf(buf, "%u", i);
  Append(buf);

  return *this;
}

String& String::operator+(const char* str)
{
  Append(str);

  return *this;
}

String& String::operator=(char c)
{
  Clear();
  Append(c);

  return *this;
}

String& String::operator=(const char* str)
{
  Clear();
  Append(str);

  return *this;
}

String& String::operator=(const String& str)
{
  // Do not copy self
  if (this != &str)
  {
    Clear();
    Append(str);
  }

  return *this;
}

bool String::operator==(const String& str) const
{
  if (!strcmp(mpChar, str.mpChar)) return TRUE;
  else                             return FALSE;
}

bool String::operator==(const char* str) const
{
  if (!strcmp(mpChar, str)) return TRUE;
  else                      return FALSE;
}

bool String::operator==(char* str) const
{
  if (!strcmp(mpChar, str)) return TRUE;
  else                      return FALSE;
}

bool String::operator!=(const char* str) const
{
  return (!(*this==str));
}

bool String::operator!=(const String& str) const
{
  return (!(*this==str));
}

void String::IncrSize()
{
  String str = *this;

  delete [] mpChar; M_UpdDestructCountN(eType_Char, mSize);

  mSize += eStringIncrSize;
  mpChar = new char[mSize]; M_UpdConstructCountN(eType_Char, mSize);
  strcpy(mpChar, str.mpChar);
}

bool String::Replace(char c, int pos)
{
  if (pos < 0) return FALSE;

  if (pos >= Length()) return FALSE;

  mpChar[pos] = c;
  return TRUE;
}

char String::Getc(int pos) const
{
  if (pos < Length())
  {
    return mpChar[pos];
  }
  else
    return (char)0;
}

bool String::Chop()
{
  int len = Length() - 1;

  if (mpChar[len] == '\n')
  {
    mpChar[len] = '\0';
    return TRUE;
  }

  return FALSE;
}

bool String::Chop(short n)
{
  int len = Length();

  if (n > len) return FALSE;

  mpChar[len-n] = '\0';

  return TRUE;
}

bool String::Find(char c) const
{
  if (strchr(mpChar, c))
    return TRUE;

  return FALSE;
}

bool String::Find(char* str) const
{
  if (str == NULL) return FALSE;

  if (strstr(mpChar, str))
    return TRUE;

  return FALSE;
}

bool String::Find(const char* str) const
{
  return Find((char*)str);
}

bool String::Find(String& str) const
{
  return Find((const char*)str);
}

unsigned String::Hash()
{
  unsigned hash = 0;
  char*    str  = mpChar;
  int c;

  while (c = *str++)
      hash = c + (hash << 6) + (hash << 16) - hash;

  return hash;
}

// End
