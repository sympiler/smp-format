//
// Created by kazem on 2020-05-17.
//

#ifndef SCO_CONVERTOR_SMP_FORMAT_H
#define SCO_CONVERTOR_SMP_FORMAT_H

#include <utility>

#include "io.h"
namespace format{
 /*
  * Storage for mat for Sparse Constraint Optimization
  * Min 1/2 x^T H x + q^T x + r
  *      A x = b
  *      l <= C x <= u
  */
 struct SMP{
  int num_vars_;
  std::string in_path_;
  std::string desc_;
  // Input attributes
  CSC *H_, *A_, *C_;
  CSC *AT_, *CT_;
  Dense  *q_, *b_, *l_, *u_;
  double r_;

  // Output attributes
  Dense *primals_, *duals_;
  double optimal_obj_;

  /*
   * Constructor for reading from file
   */
  explicit  SMP(std::string path):in_path_(std::move(path)), H_(NULLPNTR),
  A_(NULLPNTR), C_(NULLPNTR), q_(NULLPNTR),
  b_(NULLPNTR), l_(NULLPNTR), u_(NULLPNTR), r_(0),AT_(NULLPNTR),CT_(NULLPNTR),
  primals_(NULLPNTR),duals_(NULLPNTR), optimal_obj_(0),num_vars_(0),desc_(""){}


  /*
   * constructor with only inputs, no file
   */
  SMP(CSC *o, Dense *q, CSC *A, CSC *C, Dense *b, Dense *l, Dense *u,
      double r, std::string  desc)
    :H_(o), A_(A), C_(C), q_(q), b_(b), l_(l), u_(u), r_(r),
    AT_(NULLPNTR),CT_(NULLPNTR),
     primals_(NULLPNTR),duals_(NULLPNTR), optimal_obj_(0),desc_(std::move(desc)),
     in_path_(""),num_vars_(0){
   set_num_vars();
  };


  ~SMP(){
   delete H_;
   delete q_;
   delete A_;
   delete b_;
   delete C_;
   delete l_;
   delete u_;
   delete AT_;
   delete CT_;
   delete primals_;
   delete duals_;
  }

  int set_num_vars(){
   num_vars_ = H_ ? H_->n : 0;
   num_vars_ = (q_ && num_vars_ == 0) ? q_->row : num_vars_;
   num_vars_ = (A_ && num_vars_ == 0) ? A_->n : num_vars_;
   num_vars_ = (C_ && num_vars_ == 0) ? C_->n : num_vars_;
   num_vars_ = (primals_ && num_vars_ == 0) ? primals_->row : num_vars_;
   num_vars_ = (duals_ && num_vars_ == 0) ? duals_->row : num_vars_;
   return num_vars_;
  }

  bool check(){
   return !(H_->m != H_->n || H_->n != A_->n || H_->n != A_->n || H_->n != C_->n
            || l_->row != u_->row || l_->row != C_->m || A_->m != b_->row);
  }

  bool load(){
   std::ifstream fin(in_path_);
   if(fin.is_open()){
    while (!fin.eof()){
     std::string line;
     std::getline(fin, line);
     for (auto i=0; i<line.length(); line[i]=tolower(line[i]),i++);
     trim(line);
     if(line == "\"quadratic\": |" && !H_) {
      read_mtx_csc_real(fin, H_);
     } else if(line == "\"linear\": |" && !q_) {
      read_mtx_array_real(fin, q_);
     } else if(line == "\"equality\": |" && !A_) {
      read_mtx_csc_real(fin, A_);
      //smp->A_=A;
     } else if(line == "\"equality bounds\": |" && !b_) {
      read_mtx_array_real(fin, b_);
      //smp->b_=b;
     } else if(line == "\"inequality l-bounds\": |" && !l_) {
      read_mtx_array_real(fin, l_);
      //smp->l_=l;
     } else if(line == "\"inequality u-bounds\": |" && !u_) {
      read_mtx_array_real(fin, u_);
      //smp->u_=u;
     } else if(line == "\"inequality\": |" && !C_) {
      read_mtx_csc_real(fin, C_);
      //smp->C_=C;
     }else if(line == "\"fixed\": |") {
      read_real_constant(fin, r_);
     } else if(line == "\"primals\": |" && !primals_) {
      read_mtx_array_real(fin, primals_);
      //smp->primals_ = primals;
     } else if(line == "\"duals\": |" && !duals_) {
      read_mtx_array_real(fin, duals_);
      //smp->duals_=duals;
     } else if(line == "\"optimal objective\": |") {
      read_real_constant(fin, optimal_obj_);
     }else if(line == "\"description\": |" || line == "\"description\":") {
      read_string(fin, desc_);
     }else if(line.find(':') != std::string::npos){
      std::cout<<"Key in "<<line <<" is unknown\n"; fin.close();
      return false;
     }
    }
    if(set_num_vars() == 0){
     std::cout<<"Warning: Input QP is all zeros \n";
    }
   } else{
    std::cout<<"input path:"<< in_path_ <<"does not exist. \n";
    return false;
   }
   fin.close();
   return true;
  }


  bool write(const std::string& out_path){
   std::ofstream fout(out_path, std::ios::out);
   if(fout.is_open()){

    fout << "\"Description\": |\n";
    print_string(desc_,fout.rdbuf());

    if(duals_){
     fout << "\"Duals\": |\n";
     print_dense(duals_->row, duals_->col, duals_->lda, duals_->a, fout.rdbuf());
    }

    if(A_){
     fout << "\"Equality\": |\n";
     print_csc(A_->m, A_->n, A_->p, A_->i, A_->x, fout.rdbuf());
    }

    if(b_){
     fout << "\"Equality bounds\": |\n";
     print_dense(b_->row, b_->col, b_->lda, b_->a, fout.rdbuf());
    }

    fout << "\"Fixed\": |\n";
    print_constant(r_,fout.rdbuf());

    if(C_){
     fout << "\"Inequality\": |\n";
     print_csc(C_->m, C_->n, C_->p, C_->i, C_->x, fout.rdbuf());
    }

    if(l_){
     fout << "\"Inequality l-bounds\": |\n";
     print_dense(l_->row, l_->col, l_->lda, l_->a, fout.rdbuf());
    }

    if(u_){
     fout << "\"Inequality u-bounds\": |\n";
     print_dense(u_->row, u_->col, u_->lda, u_->a, fout.rdbuf());
    }

    if(q_){
     fout << "\"Linear\": |\n";
     print_dense(q_->row, q_->col, q_->lda, q_->a, fout.rdbuf());
    }

    if(primals_ || duals_){
     fout << "\"Optimal objective\": |\n";
     print_constant(optimal_obj_,fout.rdbuf());
    }

    if(primals_){
     fout << "\"Primals\": |\n";
     print_dense(primals_->row, primals_->col, primals_->lda, primals_->a,
                 fout.rdbuf());
    }

    if(H_){
     fout << "\"Quadratic\": |\n";
     print_csc(H_->m, H_->n, H_->p, H_->i, H_->x, fout.rdbuf());
    }


   } else{
    std::cout<<"The path: "<< out_path <<" is not available for writing.\n";
    return false;
   }
   fout.close();
   return true;
  }

 };
}
#endif //SCO_CONVERTOR_SMP_FORMAT_H
