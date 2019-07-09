use std::path::PathBuf;
use zokrates_core::ir;
use zokrates_field::{Field, BN128};

#[derive(Serialize, Deserialize)]
pub struct Tests {
    pub entry_point: PathBuf,
    pub tests: Vec<Test>,
}

#[derive(Serialize, Deserialize)]
pub struct Input {
    pub values: Vec<Val>,
}

#[derive(Serialize, Deserialize)]
pub struct Test {
    pub input: Input,
    pub output: TestResult,
}

pub type TestResult = Result<Output, ir::Error>;

#[derive(PartialEq, Debug)]
pub struct ComparableResult(Result<Vec<BN128>, ir::Error>);

#[derive(Serialize, Deserialize)]
pub struct Output {
    values: Vec<Val>,
}

type Val = String;

impl From<ir::ExecutionResult<BN128>> for ComparableResult {
    fn from(r: ir::ExecutionResult<BN128>) -> ComparableResult {
        ComparableResult(r.map(|v| v.return_values()))
    }
}

impl From<TestResult> for ComparableResult {
    fn from(r: TestResult) -> ComparableResult {
        ComparableResult(r.map(|v| {
            v.values
                .iter()
                .map(|v| BN128::try_from_dec_str(v).unwrap())
                .collect()
        }))
    }
}

pub fn compare(
    result: ir::ExecutionResult<BN128>,
    expected: TestResult,
) -> Result<(), String> {
    // extract outputs from result
    let result = ComparableResult::from(result);
    // deserialize expected result
    let expected = ComparableResult::from(expected);

    if result != expected {
        return Err(format!(
            "Expected {:?} but found {:?}",
            expected.0, result.0
        ));
    }

    Ok(())
}
