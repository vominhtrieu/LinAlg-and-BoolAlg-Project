﻿#include "Find_Prime_Implicants.h"

string ImplicantToString(const Implicant &imp, const vector<char> &vars_names)
{
	string result;
	for (int i = 0; i < imp.size(); i++)
	{
		if (imp.bits[i] == 1)
			result += vars_names[i] + string(".");
		else if (imp.bits[i] == 0)
			result += string("!") + vars_names[i] + string(".");
	}
	result.pop_back();

	return result;
}

void print_minterm(const VarsValue &minterm);

// hàm tính chân trị của một biểu thức hậu tố, giá trị của các biến được cho bởi vars_value
bool calculate(const string &postfix_expr, const VarsValue &vars_value, unordered_map<char, int> &vars_idx)
{	// hàm này cần được tối ưu kỹ vì sẽ được gọi 2^n lần
	// trong quá trình tính toán, lưu các giá trị trung gian trung gian trong một stack
	// nếu gặp toán tử thì pop 1 hoặc 2 toán hạng đầu stack ra, tính rồi push vào
	stack<bool> value;
	bool x, y;
	for (char c : postfix_expr)
		switch (c) {
		case '!':
			value.top() = !value.top(); break;
		case '.':
			x = value.top(); value.pop();
			y = value.top(); value.pop();
			value.push(x && y); break;
		case '+':
			x = value.top(); value.pop();
			y = value.top(); value.pop();
			value.push(x || y); break;
		case '0': case '1':
			value.push(c == '1'); break;
		default:
			value.push(vars_value[vars_idx[c]]);
		}
	// giá trị cuối cùng còn lại trong stack chính là giá trị của biểu thức cần tính
	return value.top();
}

void generate_evaluate(VarsValue &vars_value, int i, const string &postfix_expr, vector<VarsValue> &minterms, unordered_map<char, int> &vars_idx)
{	// hàm đệ quy sinh lần lượt các bộ giá trị của các biến
	static int minterm_id = 0;
	if (i == vars_value.size()) {
		cout << minterm_id++ << ":\t"; print_minterm(vars_value);
		// khi đã điền đủ các giá trị cho các biến thì thực hiện tính chân trị của biểu thức
		bool calc_out = calculate(postfix_expr, vars_value, vars_idx);	
		cout << "| " << calc_out;
		if (calc_out == 1) {
			cout << " *\n";
			// nếu chân trị bằng 1 thì bộ giá trị đó là 1 minterm, lưu lại
			minterms.push_back(vars_value);	
		}
		else cout << endl;
		return;
	}
	// 2 lời gọi đệ quy ứng với 2 cách điền giá trị cho biến thứ i
	vars_value[i] = 0;
	generate_evaluate(vars_value, i + 1, postfix_expr, minterms, vars_idx);
	vars_value[i] = 1;
	generate_evaluate(vars_value, i + 1, postfix_expr, minterms, vars_idx);
}

void generate_evaluate2(VarsValue &vars_value, int i, const string &postfix_expr, vector<VarsValue> &minterms, unordered_map<char, int> &vars_idx)
{	// hàm này giống hàm trên nhưng không in các bộ giá trị ra, dùng trong trường hợp nhiều hơn 7 biến
	static int minterm_id = 0;
	if (i == vars_value.size()) {
		if (calculate(postfix_expr, vars_value, vars_idx) == 1)
			minterms.push_back(vars_value);
		return;
	}
	vars_value[i] = 0;
	generate_evaluate2(vars_value, i + 1, postfix_expr, minterms, vars_idx);
	vars_value[i] = 1;
	generate_evaluate2(vars_value, i + 1, postfix_expr, minterms, vars_idx);
}

void find_minterms(const string &postfix_expr, vector<VarsValue> &minterms, unordered_map<char, int> &vars_idx)
{	// hàm này bao ngoài hàm đệ quy và chuẩn bị sẵn một mảng chứa các minterm tìm được
	VarsValue vars_value(vars_idx.size());
	if (vars_idx.size() <= 7) {
		cout << "\n\n=== Truth Table ===\n\t";
		for (auto &var_name : vars_idx)
			cout << var_name.first << " ";
		cout << "| func\n";
		generate_evaluate(vars_value, 0, postfix_expr, minterms, vars_idx);
	}
	else generate_evaluate2(vars_value, 0, postfix_expr, minterms, vars_idx);
}

int is_combinable_or_same(const Implicant &i1, const Implicant &i2)
{	// 0 = not combinable, 1 = combinable, 2 = same
	if (abs(i1.bit1count - i2.bit1count) > 1)
		return 0;
	int differ = 0;
	for (int i = 0; i < i1.size(); i++) {
		differ += i1[i] != i2[i];
		if (differ > 1)
			return 0;
	}
	if (differ == 1)
		return 1;
	else return 2;	// differ = 0
}

void find_prime_implicants(vector<Implicant> &implicants, vector<Implicant> &prime_implicants)
{	// hàm tìm các PI từ Implicants bậc 0 (được chuyển từ minterms thành)
	vector<Implicant> higher_order_implicants;
	while (true)
	{	// duyệt qua tất cả các cặp implicant
		for (int i = 0; i < implicants.size() - 1; i++)
			for (int j = i + 1; j < implicants.size(); j++)
			{	// kiểm tra xem chúng có ghép được với nhau ko?
				int check = is_combinable_or_same(implicants[i], implicants[j]);
				switch (check) {
				case 1:	// nếu được thì tạo một Imp bậc cao hơn và lưu vào mảng, đồng thời đánh dấu lại là đã ghép (tức ko phải prime)
					higher_order_implicants.push_back(Implicant(implicants[i], implicants[j]));
					implicants[i].is_prime = implicants[j].is_prime = 0;

					break;
				case 2:
				// trong khi ghép sẽ có trường hợp 2 implicant giống nhau, nếu phải vậy thì xóa bớt đi 1 cái
					implicants.erase(implicants.begin() + j);
					j--; break;
				}
			}
		// nếu còn tìm được các cặp ghép, thì lại duyệt trên mảng các implicant bậc cao đó để ghép tiếp
		if (higher_order_implicants.size() != 0) {
			// lấy những imp không ghép được cho vào mảng kết quả Prime Implicants
			for (auto i : implicants)
				if (i.is_prime == 1)
					prime_implicants.push_back(i);
			// tráo lại mảng để thực hiện lại vòng lặp
			implicants = higher_order_implicants;
			higher_order_implicants.clear();
		}
		else {
		// nếu ko tìm được nữa thì đem hết các implicants còn lại vào mảng kết quả
			prime_implicants.insert(prime_implicants.end(), implicants.begin(), implicants.end());
			break;
		}
	}
}

void print_minterm(const VarsValue &minterm)
{
	for (bool x : minterm)
		cout << x << " ";
}

void print_minterms(const vector<VarsValue> &minterms)
{
	for (VarsValue minterm : minterms)
	{
		for (bool x : minterm)
			cout << x << " ";
		cout << endl;
	}
}

void print_implicants(const vector<Implicant> &implicants, const vector<char> &vars_names)
{
	cout << "\n\n=== Prime implicants ===\n";
	int max_length = max_element(implicants.begin(), implicants.end(),
								[](const Implicant &im1, const Implicant &im2)
									{return im1.minterms_idx.size() < im2.minterms_idx.size(); } 
								)->minterms_idx.size();
	max_length = (max_length + 1) * 4;
	for (const auto &im : implicants)
	{
		string minterms_idx;
		for (const int &x : im.minterms_idx)
			minterms_idx += to_string(x) + string(", ");
		minterms_idx.pop_back(); minterms_idx.pop_back();
		cout << left << setw(max_length) << minterms_idx;
		
		for (const char &x : im.bits)
			if (x == -1)
				cout << "_";
			else cout << (int)x;
		
		cout << "\t\t" << ImplicantToString(im, vars_names) << endl;
	}
}
