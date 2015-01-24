
#ifndef swift_syntax_ast_consumer_hh
#define swift_syntax_ast_consumer_hh

namespace swift {
namespace ast {
class context;

class consumer {
public:
  virtual ~consumer() = default;
  virtual void process_translation_unit(context &) = 0;
};
}
}

#endif

