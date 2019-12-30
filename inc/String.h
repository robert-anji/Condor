// $RCSfile: String.h,v $
// $Revision: 1.2 $

#ifndef StringClass
#define StringClass

#include "Global.h"

enum eStringSize
{
  eStringInitSize = 0,
  eStringIncrSize = 256
};

class String
{
public:
  /* Constructors */
  String();
  String(char* str);
  String(const char* str);
  String(const String& str);
  /* Constructors to create sub-strings */
  String(String& str, short startpos);
  String(String& str, short startpos, short endpos);
  String(String& str, char* startstr);
  String(String& str, String& startstr);

  ~String();

  /* Basic functions */
  int Length(void) const;
  void Clear(void);
  char Getc(int pos) const;
  bool Chop();    // remove trailing '\n' if present
  bool Chop(short n);  // remove last 'n' characters

  /* Append and Prepend */
  void Prepend(char* str);
  void Append(const String& str);
  void Append(char* str);
  void Append(char c);
  void Append(const char* str);

  /* Insert */
  void Insert(char* str, int pos);
  void Insert(char c, int pos);
  void Insert(char c, int pos, int count);

  /* Replace */
  short Replace(char c, char d);
  short Replace(char* str1, const char* str2);
  short Replace(char* str1, const char* str2, int n);
  bool  Replace(char c, int pos);

  /* Search */
  int Position(char c)                    const;
  int Position(char c, int n)             const;
  int Position(const char* str)           const;
  int Position(const char* str, int n)    const;
  int Position(int pos, char c)           const;
  int Position(int pos, char c, int n)    const;
  int Position(int pos, const char* str)  const;
  int Position(int pos, const char* str, int n) const;
  //
  short Occurences(char c)                const;
  short Occurences(char* str)             const;
  short Occurences(const char* str)       const;
  //
  bool  Find(char c)                      const;
  bool  Find(char* str)                   const;
  bool  Find(const char* str)             const;
  bool  Find(String& str)                 const;

  /* Overloaded operators */
//operator char*()        const;
  operator const char*()  const;

  String& operator<<(char        c);
  String& operator<<(char*       str);
  String& operator<<(const char* str);
  String& operator<<(String&     str);
  // Convert number to string, append
  String& operator<<(short    i);
  String& operator<<(int      i);
  String& operator<<(unsigned i);
  //
  String& operator+ (const char*   str);
  bool    operator==(const String& str) const;
  bool    operator==(const char*   str) const;
  bool    operator==(      char*   str) const;
  bool    operator!=(const String& str) const;
  bool    operator!=(const char*   str) const;
  String& operator= (const String& str);
  String& operator= (const char*   str);
  String& operator= (char          c);

  unsigned Hash();

  // Inline
//void Print(FILE* fp) { fprintf(fp, "%s\n", string); }

private:
  void IncrSize(void);

  char* mpChar;
  short mSize;
};

#endif

// End
