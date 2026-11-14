//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// DLS Demo
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>

#define IMPLEMENT_DLS
#include "DirtyLittleScript.h"



using namespace DLS;

// includes...
#include "core/ArenaAllocator.h"
#include "core/Marshall.h"
// add a cleanup ..
ArenaAllocator Cleanup;
// add a struct
struct TestPoint {
    double x;
    double y;
};
U16 TestPointTranslatorID = 0;

void RegisterTest() {
    using namespace FunctionMap;
    // setup Marshall
    // // FIXME CurrentVM
    // // auto* testPointTranslator = new Marshall::GenericStructTranslator<TestPoint>();
    // // Cleanup.Add(testPointTranslator);
    // // testPointTranslator->registerField("x", &TestPoint::x);
    // // testPointTranslator->registerField("y", &TestPoint::y);
    // // TestPointTranslatorID = RegisterStructTranslator(testPointTranslator);
    // // FunctionMap::RegisterFunction("Point::new", [](std::span<Value> args, Value& ret) -> bool {
    // //     TestPoint* p = new TestPoint();
    // //     DLS::GarbageList.Add(p);
    // //     ret = Value(p);
    // //     ret.marshallId = TestPointTranslatorID;
    // //     if (args.size() > 0 && args[0].isArray()) {
    // //         DLS::Marshall::IStructTranslator* tl = DLS::Marshall::getStructTranslator(ret);
    // //         if (tl) tl->saveArrayValue(ret,Globals::CurrentVM->Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
    // //     }
    // //     return true;
    // // }, "Create a Point");

}



int main(int argc, char* argv[]) {

    DLS::Init(argc, argv);
    RegisterTest();

#ifdef DLS_DEBUG
    assert(sizeof(Value) == 16 && "Check Value! It's not 16 byte!");
#endif
    int minargs = 1; // for more params
    if (argc > minargs) {
        Run(loadScript( argv[minargs]));
        return 0;
    }

    std::string mySrc = "";
    std::cout << "============= Dirty Little Script ===============\n";
// goto jump_to_test;

    std::cout << "Value size check:" << sizeof(Value) << "\n";
    std::cout << "ValueType size check:" << sizeof(ValueType) << "\n";

    std::cout << "=============  TAPE TEST ===============\n";
    std::cout << "------------ String  ---------------\n";
    mySrc = R"( "Hello World!" )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";

    std::cout << "------------ BASIC MATH ---------------\n";
    mySrc = R"( (4.f + 5) * 20 )";
    std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( 3 * 4 + 1 )";
    std::cout << mySrc << " =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( 3 * (4 + 1) )";
    std::cout << mySrc << " = " << Run(mySrc).toString() << "\n";
    std::cout << "------------ (multi result) ---------------\n";
    mySrc = R"( 3 * (4 + 1); 5 + 5 )";
    std::cout << mySrc << " = " << Run(mySrc).toString() << "\n";

    std::cout << "------------ C-Function Call ---------------\n";
    mySrc = R"( return math::randomf )"; std::cout << mySrc << " = " << Run(mySrc).toString() << "\n";
    mySrc = R"( math::random 10 20 )"; std::cout << mySrc << " = " << Run(mySrc).toString() << "\n";

    // x = fun not supported !
    mySrc = R"( fun = function; print "INSIDE";; fun; print "OUTSIDE" )"; std::cout << mySrc << " = " << Run(mySrc).toString() << "\n";



    std::cout << "------------ Comparison ---------------\n";
    mySrc = R"( (4.f + 5) > 20 )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( (4.f + 5) < 20 )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( (4.f + 5) == 20 )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( (4.f + 5) != 20 )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( (4.f + 5) <= 20 )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( (4.f + 5) >= 20 )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";

    std::cout << "------------ If ---------------\n";
    mySrc = R"( if 1 > 2; 40; end; )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( if 1 > 2; 40; else 20; end; )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( if 1 > 2; 40; elif 2 > 1; 100; else; 20; end; )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    mySrc = R"( if 1 < 2; 40; end; )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    //FIXME not causing assert .. i mixed something up in parser. should be parse error!
    // mySrc = R"( if; 1 < 2; 40; end; )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";
    std::cout << "------------ Semicolon test  ---------------\n";
    // skip empty:
    mySrc = R"( ;v = 50; )"; std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    // semi + linebreak
    mySrc = R"(
        if 1 > 2;
            50;
        elif 2 > 1;
            100;
        else
            150;
        ;;
    )";
    std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";
    // only linebreak
    mySrc = R"(
        if 1 > 2
            50
        elif 2 == 2
            100
        else
            150
        ;;
    )";
    std::cout << mySrc << "  =  " << Run(mySrc).toString() << "\n";

    std::cout << "---- nested func call and reverse POP_STACK and request params ----\n";
    // mySrc = R"( print (math::randomf) (math::rand);)"; Run(mySrc);
    mySrc = R"( print "Hello" "World" 47.11 815 )";
    Tools::printf("%s :: RESULT=%s\n", mySrc.c_str(),Run(mySrc).toString().c_str());
    mySrc = R"( print math::randomf, math::rand;)";
    Tools::printf("%s :: RESULT=%s\n", mySrc.c_str(),Run(mySrc).toString().c_str());
    mySrc = R"( print "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" )";
    Tools::printf("%s :: RESULT=%s\n", mySrc.c_str(),Run(mySrc).toString().c_str());

    std::cout << "---- Global Variables ----\n";
    mySrc = R"( a = 12.0; print a; )"; Run(mySrc);
    mySrc = R"( i = 24.0;print i;  i = i + 1; print "i ==" i (i < 10); )"; Run(mySrc);

    std::cout << "---- While ----\n";
    mySrc = R"( itr = 0;while itr < 10; itr = itr + 1; print "iter=" itr;;  )"; Run(mySrc);
    mySrc = R"(
# a Comment!
        itr = 10
        while itr > 0
            itr = itr - 1
            print "iter=" itr
            break
        end
    )"; Run(mySrc);


   std::cout << "---- Array ----\n";
    mySrc = R"( foo[110] = 10; print "10==" foo[110] )"; Run(mySrc);

    std::cout << "------- Modulo --------\n";
    mySrc = R"( i = 0; while i < 10; if i % 2; print "mod2!" i;; i = i + 1;; )"; Run(mySrc);

    std::cout << "-------- And / Or --------\n";
    mySrc = R"( print 0 || 1 )"; Run(mySrc);
    mySrc = R"( i = 10 ; if i == 0 || i == 10; print "i is:" i;else print "not"; end  )"; Run(mySrc);
    mySrc = R"( i = 10 ; if i > 9 && i < 20; print "i is " i; else print "not";;  )"; Run(mySrc);
    mySrc = R"( i = 10 ; if i == 0 || i == 9; print "i is:" i;else print "not"; end  )"; Run(mySrc);
    mySrc = R"( i = 10 ; if i > 11 && i < 20; print "i is " i; else print "not";;  )"; Run(mySrc);

// DEBUG_TOKEN = true;
    std::cout << "-------- Lambda Fn --------\n";
    mySrc = R"(
        foo = function ; print str::strlen "HUHU";;
        foo
        return
        print "still here ?"
    )"; Run(mySrc);
// DEBUG_TOKEN = false;

std::cout << "-------- Array again--------\n";
mySrc = R"(
i = 0
minW = -200
maxW = 200
minH = -100
maxH = 100
while i < 1;
    Points_X[i] =  math::random minW maxW
    print i ":X:" Points_X[i]
    Points_Y[i] =  math::random minH maxH
    print i ":X Y:" Points_X[i] Points_Y[i]
    Points_Z[i] =  math::randomf
    print i ":Z:" Points_Z[i]
    print i ":X Y Z:" Points_X[i] Points_Y[i] Points_Z[i]
    i = i + 1
end

)"; Run(mySrc);

// DEBUG_TOKEN = true;
std::cout << "-------- Comment test  --------\n";
mySrc = R"(
    foo = 1
    bar = 2
    print "#" foo bar
)"; Run(mySrc);

std::cout << "-------- Basic Commands --------\n";
mySrc = R"(
    arr[999] = 1; print "array::count arr=" array::count arr
    print "cast int:" int::cast 47.11
    print "cast float:" float::cast 10
    print "get variable type" getType arr
)"; Run(mySrc);

mySrc = R"(
    foo = 47.11;
    arr[999] = 1; print "array::count arr=" array::count arr
    print "cast int:" int::cast 47.11
    print "cast float:" float::cast 10
    print "get variable type" getType arr
    print " PARSE ERROR expected:"
    fn
)"; Run(mySrc);


mySrc = R"(
    i = 0
    while i < 10
        r[0] = math::random 0 10
        r[1] = math::random 0 10
        Points[i] = r

        i = i + 1
    end
    i = 0
    while i < 10
        r = Points[i]
        print r[0] r[1]
        i = i + 1
    end

)"; Run(mySrc);

mySrc = R"(
    print "parse error? or endless loop"
    i = 0; while i < 100; i++;;
)"; Run(mySrc);



mySrc = R"(
    arr[0] = { 10 20 math::rand}
    print 1
    item = arr[0]
    print 2
    print "item type:" (getType item) "arr type:" (getType arr)
    print "0=" item[0] "1=" item[1] "2=" item[2]
    item[0] = 100
    print 3
    arr[0] = item
    print 4
    item = arr[0]
    print 5
)";
Run(mySrc);



mySrc = R"(
    arr = { 10 20 math::rand}
    forRange i 3; print i arr[i] ;;

    forRange i 3
       arr[i] = { 10 20 30 }
    end

    forRange i 3
       p = arr[i]
       print p[0] p[1] p[2]
    end


)";
Run(mySrc);


mySrc = R"(
    arr[0] = { 10 20 30 }
    print arr[0][2]
    arr[1] = { 40 50 60 }
    print arr[1][0]
    arr[1][0]  = 4711
    print arr[1][0]
)";
Run(mySrc);

mySrc = R"(
    arr[0] = { 10 20 30 }
    arr[0][4]  = 815
    print arr[0][4]
)";
Run(mySrc);

jump_to_test:
mySrc = R"(
    a = 0
    a++
    print a
    a--
    print a
)";
Run(mySrc);

mySrc = R"(
    help
    print sys::argc
    forRange i sys::argc; print "#" i (sys::argv i);;
    print "--- Test struct ---"
    p = Point::new {10.1 20.2}
    print p.x p.y
)";
Run(mySrc);

// goto jump_to_end;
jump_to_end:
Tools::printf("");
}
