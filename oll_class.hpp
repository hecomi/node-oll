#include <string>
#include <iostream>
#include <boost/optional.hpp>
#include "oll.hpp"

/**
 *  オンライン学習ライブラリの機能をまとめたクラス
 *  @template TrainMethodNum oll_tool::学習手法（P, AP, PA, PA1, PA2, PAK, CW, AL）
 */
template <typename TrainMethod = oll_tool::PA1_s>
class OLL
{
public:
	/**
	 *  コンストラクタ
	 *  @param[in] C    Regularization Parameter
	 *  @param[in] bias Bias
	 */
	OLL(float C = 1.f, float bias = 0.f)
	{
		ol_.setC(C);
		ol_.setBias(bias);
	}

	/**
	 *  学習結果をファイルに保存
	 *  @param[in] file_name 保存先ファイル名
	 */
	bool save(const std::string& file_name)
	{
		if ( ol_.save(file_name.c_str()) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return false;
		}
		return true;
	}

	/**
	 *  学習結果をファイルから復元
	 *  @param[in] file_name 復元元ファイル名
	 */
	bool load(const std::string& file_name)
	{
		if ( ol_.load(file_name.c_str()) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return false;
		}
		return true;
	}

	/**
	 *  データを渡して学習させる（正例、負例別）
	 *  @param[in] flag true: +のデータ、false: -のデータ
	 *  @param[in] data 学習データ (format: id:val id:val ...)
	 */
	bool add(bool flag, const std::string& data)
	{
		std::string format = ( flag ? "+1 " : "-1 " ) + data;
		oll_tool::fv_t fv;
		int y = 0;

		if (ol_.parseLine(format, fv, y) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return false;
		}

		ol_.trainExample(TrainMethod(), fv, y);
		return true;
	}

	/**
	 *  データを渡して学習させる（正例、負例込）
	 *  @param[in] data 学習データ (format: (1|-1) id:val id:val ...)
	 */
	bool add(const std::string& data)
	{
		oll_tool::fv_t fv;
		int y = 0;

		if (ol_.parseLine(data, fv, y) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return false;
		}

		ol_.trainExample(TrainMethod(), fv, y);
		return true;
	}

	/**
	 *  データをテストする
	 *  @param[in] data テストデータ : id:val id:val ...
	 */
	float test(const std::string& data)
	{
		std::string format = "0 " + data;
		oll_tool::fv_t fv;
		int y = 0;

		if (ol_.parseLine(format, fv, y) == -1) {
			std::cerr << ol_.getErrorLog() << std::endl;
			return -1;
		}

		return ol_.classify(fv);
	}

private:
	//! オンライン学習するクラス
	oll_tool::oll ol_;
};

