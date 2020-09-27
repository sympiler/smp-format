//
// Created by kazem on 6/12/20.
//

#include "utils.h"
#include "mp_format_converter.h"
#include "exceptions.h"


using namespace format;

int main(int argc, const char *argv[]){

 std::map<std::string,std::string> qp_args;

 try {
  parse_args_bounded(argc, argv, qp_args);
 } catch (const missing_arg_error& e) {
  std::cerr << "Error: Missing argument: " << e.arg() << std::endl;
  exit(1);
 } catch (...) {
  std::cerr << "An unknown error has occurred while parsing" << std::endl;
  exit(1);
 }

 std::string p_name, q_name, l_name, a_name, u_name;
 std::string output = "noname.yml";

 if(qp_args.find("quadratic") != qp_args.end())
  p_name = qp_args["quadratic"];
 if(qp_args.find("linear") != qp_args.end())
  q_name = qp_args["linear"];
 if(qp_args.find("l-bounds") != qp_args.end())
  l_name = qp_args["l-bounds"];
 if(qp_args.find("constraints") != qp_args.end())
  a_name = qp_args["constraints"];
 if(qp_args.find("u-bounds") != qp_args.end())
  u_name = qp_args["u-bounds"];
 if(qp_args.find("output") != qp_args.end())
  output = qp_args["output"];

 BoundedForm *bf = NULLPNTR;
 try {
  bf = load_bounded(p_name, q_name, l_name, a_name, u_name);
 } catch (const read_file_error& e) {
  std::cerr << "Error: Cannot read file: " << e.filename() << std::endl;
  delete bf;
  exit(1);
 } catch (const mtx_format_error& e) {
  std::cerr << "Error:\tMatrix format mismatch in " << e.filename() << std::endl
            << "\tExpecting " << e.expected_format()
            << " but got " << e.got_format()
            << std::endl;
  delete bf;
  exit(1);
 } catch (const mtx_arith_error& e) {
  std::cerr << "Error:\tMatrix arithmetic mismatch in "
            << e.filename() << std::endl
            << "\tExpecting " << e.expected_arith()
            << " but got " << e.got_arith()
            << std::endl;
  delete bf;
  exit(1);
 } catch (const mtx_header_error& e) {
  std::cerr << "Error:\tInvalid header in " << e.filename() << std::endl
            << "\t" << e.what() << std::endl;
  delete bf;
  exit(1);
 } catch (...) {
  std::cerr << "An unknown error has occurred while loading" << std::endl;
  delete bf;
  exit(1);
 }

 Description test;
 std::string dtes=test.get_desc();
 auto *qfc = new QPFormatConverter(bf);

 qfc->bounded_to_smp();
 try {
 qfc->smp_->write(output);
 } catch (const write_file_error& e) {
  std::cerr << "Error: Unable to write to file " << e.filename();
  delete bf;
  delete qfc;
  exit(1);
 } catch (...) {
  std::cerr << "An unknown error has occurred while writing" << std::endl;
  delete bf;
  delete qfc;
  exit(1);
 }

 delete bf;
 delete qfc;

 return 0;
}
