
#ifndef __COSIN_DATABASE_DB_EXCEPTION_H_
#define __COSIN_DATABASE_DB_EXCEPTION_H_

#include "cosin/exception.h"

namespace cosin
{
namespace database
{

class db_exception : public exception<int>
{
private:
typedef exception<int>		base_type;
typedef db_exception		this_type;
};

}
}

#endif
