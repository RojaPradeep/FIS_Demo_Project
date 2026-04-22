#pragma once
#include <string>
using namespace std;

enum class Status
{
   NEW,
   VALIDATED,
   PROCESSED,
   SETTLED,
   REJECTED
};

struct Trade
{
   int id;
   string type;
   double amount;
   Status status{Status::NEW};
};
