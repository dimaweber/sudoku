#include "test_cell.cpp"
#include "test_coord.cpp"

int main(int argc, char** argv)
{
   int status = 0;
   {
      CellTest tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      CoordTest tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   return status;
}
