#include "taichi/ir/expr.h"

#include "taichi/ir/frontend_ir.h"
#include "taichi/ir/ir.h"
#include "taichi/program/program.h"

TLANG_NAMESPACE_BEGIN

void Expr::serialize(std::ostream &ss) const {
  TI_ASSERT(expr);
  expr->serialize(ss);
}

std::string Expr::serialize() const {
  std::stringstream ss;
  serialize(ss);
  return ss.str();
}

void Expr::set_tb(const std::string &tb) {
  expr->tb = tb;
}

void Expr::set_attribute(const std::string &key, const std::string &value) {
  expr->set_attribute(key, value);
}

std::string Expr::get_attribute(const std::string &key) const {
  return expr->get_attribute(key);
}

DataType Expr::get_ret_type() const {
  return expr->ret_type;
}

void Expr::type_check(CompileConfig *config) {
  expr->type_check(config);
}

Expr cast(const Expr &input, DataType dt) {
  return Expr::make<UnaryOpExpression>(UnaryOpType::cast_value, input, dt);
}

Expr bit_cast(const Expr &input, DataType dt) {
  return Expr::make<UnaryOpExpression>(UnaryOpType::cast_bits, input, dt);
}

Expr Expr::operator[](const ExprGroup &indices) const {
  TI_ASSERT(is<GlobalVariableExpression>() || is<ExternalTensorExpression>());
  return Expr::make<GlobalPtrExpression>(*this, indices);
}

Expr &Expr::operator=(const Expr &o) {
  set(o);
  return *this;
}

Expr Expr::parent() const {
  TI_ASSERT_INFO(is<GlobalVariableExpression>(),
                 "Cannot get snode parent of non-global variables.");
  return Expr::make<GlobalVariableExpression>(
      cast<GlobalVariableExpression>()->snode->parent);
}

SNode *Expr::snode() const {
  TI_ASSERT_INFO(is<GlobalVariableExpression>(),
                 "Cannot get snode of non-global variables.");
  return cast<GlobalVariableExpression>()->snode;
}

Expr Expr::operator!() {
  return Expr::make<UnaryOpExpression>(UnaryOpType::logic_not, expr);
}

void Expr::declare(DataType dt) {
  set(Expr::make<GlobalVariableExpression>(dt, Identifier()));
}

void Expr::set_grad(const Expr &o) {
  this->cast<GlobalVariableExpression>()->adjoint.set(o);
}

Expr::Expr(int16 x) : Expr() {
  expr = std::make_shared<ConstExpression>(PrimitiveType::i16, x);
}

Expr::Expr(int32 x) : Expr() {
  expr = std::make_shared<ConstExpression>(PrimitiveType::i32, x);
}

Expr::Expr(int64 x) : Expr() {
  expr = std::make_shared<ConstExpression>(PrimitiveType::i64, x);
}

Expr::Expr(float32 x) : Expr() {
  expr = std::make_shared<ConstExpression>(PrimitiveType::f32, x);
}

Expr::Expr(float64 x) : Expr() {
  expr = std::make_shared<ConstExpression>(PrimitiveType::f64, x);
}

Expr::Expr(const Identifier &id) : Expr() {
  expr = std::make_shared<IdExpression>(id);
}

Expr expr_rand(DataType dt) {
  return Expr::make<RandExpression>(dt);
}

Expr snode_append(SNode *snode, const ExprGroup &indices, const Expr &val) {
  return Expr::make<SNodeOpExpression>(snode, SNodeOpType::append, indices,
                                       val);
}

Expr snode_append(const Expr &expr, const ExprGroup &indices, const Expr &val) {
  return snode_append(expr.snode(), indices, val);
}

Expr snode_is_active(SNode *snode, const ExprGroup &indices) {
  return Expr::make<SNodeOpExpression>(snode, SNodeOpType::is_active, indices);
}

Expr snode_length(SNode *snode, const ExprGroup &indices) {
  return Expr::make<SNodeOpExpression>(snode, SNodeOpType::length, indices);
}

Expr snode_get_addr(SNode *snode, const ExprGroup &indices) {
  return Expr::make<SNodeOpExpression>(snode, SNodeOpType::get_addr, indices);
}

Expr snode_length(const Expr &expr, const ExprGroup &indices) {
  return snode_length(expr.snode(), indices);
}

Expr assume_range(const Expr &expr, const Expr &base, int low, int high) {
  return Expr::make<RangeAssumptionExpression>(expr, base, low, high);
}

Expr loop_unique(const Expr &input, const std::vector<SNode *> &covers) {
  return Expr::make<LoopUniqueExpression>(input, covers);
}

Expr global_new(Expr id_expr, DataType dt) {
  TI_ASSERT(id_expr.is<IdExpression>());
  auto ret = Expr(std::make_shared<GlobalVariableExpression>(
      dt, id_expr.cast<IdExpression>()->id));
  return ret;
}

Expr global_new(DataType dt, std::string name) {
  auto id_expr = std::make_shared<IdExpression>(name);
  return Expr::make<GlobalVariableExpression>(dt, id_expr->id);
}
TLANG_NAMESPACE_END
