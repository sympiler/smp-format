//
// Created by kazem on 3/6/19.
//
#include "qp_format_converter.h"
#include "../mp_format_converter.h"

using namespace old;
format::CSC *old_to_new(CSC *in_csc){
 if(!in_csc)
  return NULLPNTR;
 format::CSC *tmp = new format::CSC;
 tmp->p = in_csc->p;
 tmp->i = in_csc->i;
 tmp->x = in_csc->x;
 tmp->nnz =in_csc->nzmax;
 tmp->m = in_csc->nrow;
 tmp->n = in_csc->ncol;
 tmp->stype = in_csc->stype == -1 ? format::LOWER : format::GENERAL;
 tmp->is_pattern = false;
 tmp->pre_alloc = true;
 return tmp;
}

format::Dense *dbl_to_dnse(double *val, int dim){
 if(!val)
  return NULLPNTR;
 auto *tmp = new format::Dense(dim,1,1);
 for (int i = 0; i < dim; ++i) {
  tmp->a[i] = val[i];
 }
 return tmp;
}

int QP_demo01(int argc, char **argv);

int main(int argc, char *argv[]) {
 QP_demo01(argc, argv);
}


void get_num_iter(double acc_thresh, int& inner_iter_ref,
                  int& outer_iter_ref){
 if(acc_thresh >= 1e-3){
  inner_iter_ref = outer_iter_ref = 1;
 } else if (acc_thresh <= 1e-4 && acc_thresh > 1e-10){
  inner_iter_ref = outer_iter_ref = 2;
 } else if (acc_thresh <= 1e-11 && acc_thresh > 1e-13){
  inner_iter_ref = outer_iter_ref = 3;
 } else{
  inner_iter_ref = outer_iter_ref = 9;
 }
}



std::string strip_name(std::string name){
 name.find(".");
 auto p1 = name.rfind("/");
 auto p2 = name.rfind("_");
 std::string slide = name.substr( p1 != std::string::npos ? p1+1 : 0,
   p2 != std::string::npos ? p2-p1-1 : std::string::npos);
 return slide;
}

/*
 *
 */
int QP_demo01(int argc, char **argv){
 if (argc < 1) {
  std::cout << "missing args!\n";
  return -1;
 }

 double reg_diag=1e-9;
 double zero_threshold = reg_diag;
 double eps = 1e-6;
 int inner_iter = 2;
 int outer_iter = 2;
 double stop_tol = 1e-15;
 int solver_mode = 1;
 std::string hessian_file = "";
 std::string linear_file = "";
 std::string ineq_file_l = "";
 std::string ineq_file = "";
 std::string ineq_file_u = "";
 std::string eq_file = "";
 std::string eq_file_u = "";
 std::string base = "/home/kazem/SMP_Repository/";
 std::string stripped_name;

 std::string group, source, application;

 QPFormatConverter *QPFC = new QPFormatConverter();

 int qp_type = atoi(argv[1]);// 1=IE format, 2 general ineq
 if(qp_type == 1){
  if (argc < 8) {
   std::cout << "missing args!\n";
   return -1;
  }
  hessian_file = argv[2];
  linear_file = argv[3];
  eq_file = argv[4];
  eq_file_u = argv[5];
  ineq_file = argv[6];
  ineq_file_u = argv[7];
  if(argc>8){
   int reg_diag_in = atoi(argv[8]);
   int outer_iter_in = atoi(argv[9])-1;
   int inner_iter_in = atoi(argv[10])-1;
   int eps_in = atoi(argv[11]);
   int tol_in = atoi(argv[12]);
   int sol_mode = 0;
   if(argc > 13)
    sol_mode = atoi(argv[13]);

   reg_diag = pow(10,-reg_diag_in);
   zero_threshold = reg_diag;
   eps = pow(10,-eps_in);
   stop_tol = pow(10,-tol_in);
   inner_iter = inner_iter_in;
   outer_iter = outer_iter_in;
   solver_mode = sol_mode;
   group = argv[14];
   source = argv[15];
   application = argv[16];
  }
  stripped_name = strip_name(hessian_file);

  QPFC->read_IE_format(hessian_file,linear_file,
                       eq_file,eq_file_u,ineq_file,
                       ineq_file_u);

  auto *ief = new format::IEForm();
  QPFC->H->nrow = QPFC->H->ncol;
  QPFC->H->stype = -1;
  ief->H = old_to_new(QPFC->H);
  ief->q = dbl_to_dnse(QPFC->q, QPFC->H->nrow);
  if(QPFC->A_eq){
   QPFC->A_eq->stype = 0;
   ief->A = old_to_new(QPFC->A_eq);
   ief->b = dbl_to_dnse(QPFC->a_eq, QPFC->A_eq->nrow);
  } else{
   ief->A = NULLPNTR;
   ief->b = NULLPNTR;
  }
  if(QPFC->A_ineq){
   QPFC->A_ineq->stype = 0;
   ief->C = old_to_new(QPFC->A_ineq);
   ief->d = dbl_to_dnse(QPFC->a_ineq, QPFC->A_ineq->nrow);
  } else{
   ief->C = NULLPNTR;
   ief->d = NULLPNTR;
  }
  auto *qfc4 = new format::QPFormatConverter(ief);
  qfc4->ie_to_smp();
  std::string smp_path = base + group + "/" + stripped_name +".yml";
  format::Description d;
  d.source_ = source; d.group_=group; d.name_ = stripped_name;
  d.application_=application;
  if(application == "Model Predictive Control")
   d.category_ = "robotics";
  qfc4->smp_->write(smp_path);
  auto *qfc5 = new format::QPFormatConverter(qfc4->smp_);
  qfc5->smp_to_ie();
  if(!sym_lib::are_equal(qfc4->ief_, qfc5->ief_) )
   std::cout<<"WRONG conversion in "<<hessian_file<<"\n";

  delete qfc4;
  delete qfc5;
  delete ief->q;
  delete ief->b;
  delete ief->d;

 } else if(qp_type==2){
  if (argc < 7) {
   std::cout << "missing args!\n";
   return -1;
  }
  hessian_file = argv[2];
  linear_file = argv[3];
  ineq_file_l = argv[4];
  ineq_file = argv[5];
  ineq_file_u = argv[6];

  if(argc>7){
   int reg_diag_in = atoi(argv[7]);
   int outer_iter_in = atoi(argv[8])-1;
   int inner_iter_in = atoi(argv[9])-1;
   int eps_in = atoi(argv[10]);
   int tol_in = atoi(argv[11]);
   int sol_mode = 0;
   if(argc > 12)
    sol_mode = atoi(argv[12]);
   reg_diag = pow(10,-reg_diag_in);
   zero_threshold = reg_diag;
   eps = pow(10,-eps_in);
   stop_tol = pow(10,-tol_in);
   inner_iter = inner_iter_in;
   outer_iter = outer_iter_in;
   solver_mode = sol_mode;
   group = argv[13];
   source = argv[14];
   application = argv[15];
  }
  stripped_name = strip_name(hessian_file);
  QPFC->read_bounded_format(hessian_file,linear_file,
                            ineq_file_l,ineq_file,ineq_file_u);
  //QPFC->print_bounded_format();


  QPFC->B2IE();
  format::BoundedForm *bf = new format::BoundedForm();
  QPFC->H->nrow = QPFC->H->ncol;
  QPFC->H->stype = -1;
  bf->H = old_to_new(QPFC->H);
  bf->q = dbl_to_dnse(QPFC->q, QPFC->H->nrow);
  if(QPFC->A){
   bf->A = old_to_new(QPFC->A);
   bf->l = dbl_to_dnse(QPFC->l, QPFC->A->nrow);
   bf->u = dbl_to_dnse(QPFC->u, QPFC->A->nrow);
  } else{
   bf->A = NULLPNTR;
   bf->l = NULLPNTR;
   bf->u = NULLPNTR;
  }
  auto *qfc6 = new format::QPFormatConverter(bf);
  qfc6->bounded_to_smp();
  format::Description d;
  d.source_ = source; d.group_=group; d.name_ = stripped_name;
  d.application_=application;
  if(application == "Maros-Meszaros")
   d.category_ = "Maros-Meszaros";
  std::string smp_path = base + group + "/" + stripped_name +".yml";
  qfc6->smp_->write(smp_path);

  delete qfc6;
  delete bf->l;
  delete bf->u;



  //QPFC->print_IE_format();
  //QPFC->IE_export_to_dense(linear_file);
  auto *ief = new format::IEForm();
  QPFC->H->nrow = QPFC->H->ncol;
  QPFC->H->stype = -1;
  ief->H = old_to_new(QPFC->H);
  ief->q = dbl_to_dnse(QPFC->q, QPFC->H->ncol);
  if(QPFC->A_eq){
   QPFC->A_eq->stype = 0;
   ief->A = old_to_new(QPFC->A_eq);
   ief->b = dbl_to_dnse(QPFC->a_eq, QPFC->A_eq->nrow);
  } else{
   ief->A = NULLPNTR;
   ief->b = NULLPNTR;
  }
  if(QPFC->A_ineq){
   QPFC->A_ineq->stype = 0;
   ief->C = old_to_new(QPFC->A_ineq);
   ief->d = dbl_to_dnse(QPFC->a_ineq, QPFC->A_ineq->nrow);
  } else{
   ief->C = NULLPNTR;
   ief->d = NULLPNTR;
  }
  auto *qfc4 = new format::QPFormatConverter(ief);
  qfc4->ie_to_smp();
  //qfc4->smp_->write("test01_0.yml");
  auto *qfc5 = new format::QPFormatConverter(qfc4->smp_);
  qfc5->smp_to_ie();
  if(!sym_lib::are_equal(qfc4->ief_, qfc5->ief_) )
   std::cout<<"WRONG conversion in "<<hessian_file<<"\n";

  delete qfc4;
  delete qfc5;
  delete ief->q;
  delete ief->b;
  delete ief->d;

 }else{
  return -1;
 }




 delete QPFC;

}
