#include <memory>
#include <node.h>
#include "oll.hpp"
#include "v8.hpp"

using namespace v8;

/**
 * JavaScript へ OLL するクラスをエクスポートするためのクラス
 */
template <typename TrainMethod>
class OllJs : public hecomi::V8::ExportToJSIF
{
private:
	//! オンライン学習してくれるクラス
	std::shared_ptr<OLL<TrainMethod>> oll_;

public:
	//! JavaScript へエクスポートする関数
	boost::any func(const std::string& func_name, const Arguments& args)
	{
		HandleScope scope;

		if (func_name == "constructor") {
			float C = 1.f, bias = 0.f;
			if (args[0]->IsObject()) {
				Local<Object> obj = Local<Object>::Cast(args[0]);
				if (obj->Get(String::New("C"))->IsNumber())
					C = static_cast<float>(obj->Get(String::New("C"))->NumberValue());
				if (obj->Get(String::New("bias"))->IsNumber())
					bias = static_cast<float>(obj->Get(String::New("bias"))->NumberValue());
			}
			oll_ = std::make_shared<OLL<TrainMethod>>(C, bias);
			return 0;
		}
		else if (func_name == "save") {
			// 引数が文字列かどうかチェック
			if (!args[0]->IsString()) {
				Local<String> msg = String::New("[MeCab] Error! Argument of 'save' must be String.");
				ThrowException(Exception::TypeError(msg));
				return false;
			}
			String::Utf8Value filename(args[0]);
			return oll_->save(*filename);
		}
		else if (func_name == "load") {
			// 引数が文字列かどうかチェック
			if (!args[0]->IsString()) {
				Local<String> msg = String::New("[OLL] Error! Argument of 'load' must be String.");
				ThrowException(Exception::TypeError(msg));
				return false;
			}
			String::Utf8Value filename(args[0]);
			return oll_->load(*filename);
		}
		else if (func_name == "add") {
			// 正例、負例込
			// 第１引数が文字列かどうかチェック
			if (args[0]->IsString()) {
				String::Utf8Value data(args[0]);
				return oll_->add(*data);
			}

			// 正例、負例別
			// 第１引数が BOOL 値かどうかチェック
			if (!args[0]->IsBoolean()) {
				Local<String> msg = String::New("[OLL] Error! The first argument of 'add' must be String or Boolean.");
				ThrowException(Exception::TypeError(msg));
				return false;
			}

			// 第２引数が文字列かどうかチェック
			if (!args[1]->IsString()) {
				Local<String> msg = String::New("[OLL] Error! The second argument of 'add' must be String.");
				ThrowException(Exception::TypeError(msg));
				return false;
			}

			String::Utf8Value data(args[1]);
			return oll_->add(args[0]->BooleanValue(), *data);
		}
		else if (func_name == "test") {
			// 第１引数が文字列かどうかチェック
			if (!args[0]->IsString()) {
				Local<String> msg = String::New("[OLL] Error! Argument of 'test' must be String.");
				ThrowException(Exception::TypeError(msg));
				return -1.0;
			}

			String::Utf8Value data(args[0]);
			return static_cast<double>(oll_->test(*data));
		}

		return 0;
	}
};

/**
 * 各種 Train Method をエクスポートする
 */
template <typename TrainMethod>
void set(Handle<Object>& target, const std::string& name)
{
	hecomi::V8::ExportToJS<OllJs<TrainMethod>> train_method(name);
	train_method.template add_func<bool>("save");
	train_method.template add_func<bool>("load");
	train_method.template add_func<bool>("add");
	train_method.template add_func<double>("test");
	target->Set(
		String::NewSymbol(name.c_str()),
		train_method.get_class()->GetFunction()
	);
}

/**
 * Node.js の世界へいってらっしゃい
 */
void init(Handle<Object> target) {
	set<oll_tool::P_s>   ( target, "P"   );
	set<oll_tool::AP_s>  ( target, "AP"  );
	set<oll_tool::PA_s>  ( target, "PA"  );
	set<oll_tool::PA1_s> ( target, "PA1" );
	set<oll_tool::PA2_s> ( target, "PA2" );
	set<oll_tool::PAK_s> ( target, "PAK" );
	set<oll_tool::CW_s>  ( target, "CW"  );
	set<oll_tool::AL_s>  ( target, "AL"  );
}

NODE_MODULE(oll, init)

