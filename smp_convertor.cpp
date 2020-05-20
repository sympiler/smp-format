//
// Created by kazem on 2020-05-09.
//

#include <iostream>
#include <map>
#include <cmath>

//#include "qp_format_converter.h"
#include "utils.h"
#include "smp_format.h"


using namespace format;

int main(int argc, char *argv[]){

 std::map<std::string,std::string> qp_args;
 parse_args(argc, argv, qp_args);

 SMP *smp = new SMP(qp_args["H"]);
 smp->load();
 std::cout<<"\n";
 smp->write("tmp.yml");
 delete smp;
 /// Reading input matrices.
// Eigen::SparseMatrix<double,Eigen::ColMajor,int> H, A, C;
// Eigen::VectorXd q, b, d;
// std::string message = "Could not load ";
// if( !Eigen::loadMarket( H, qp_args["H"] ) ){ std::cout<<message<<"H"; return 1; }
// if( !Eigen::loadMarketVector( q, qp_args["q"] ) ){ std::cout<<message<<"q"; return 1; }
// if( !Eigen::loadMarket( A, qp_args["A"] ) ){ std::cout<<message<<"A"; return 1; }
// if( !Eigen::loadMarketVector( b, qp_args["b"] ) ){ std::cout<<message<<"b"; return 1; }
// if( !Eigen::loadMarket( C, qp_args["C"] ) ){ std::cout<<message<<"C"; return 1; }
// if( !Eigen::loadMarketVector( d, qp_args["d"] ) ){ std::cout<<message<<"d"; return 1; }

 return 0;
}