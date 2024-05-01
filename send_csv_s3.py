import boto3
from botocore.exceptions import NoCredentialsError
from dotenv import load_dotenv
import os

load_dotenv()

aws_access_key_id = os.getenv("AWS_ACCESS_KEY_ID")
aws_secret_access_key = os.getenv("AWS_SECRET_ACCESS_KEY")
bucket_name = os.getenv("AWS_BUCKET_NAME")

folder_path = "."
csv_files = [file for file in os.listdir(folder_path) if file.endswith('.csv')]

def upload_to_s3(file_path, bucket_name, object_name):
    s3 = boto3.client("s3", aws_access_key_id=aws_access_key_id, aws_secret_access_key=aws_secret_access_key)

    try:
        s3.upload_file(file_path, bucket_name, object_name)
    except FileNotFoundError:
        print("O arquivo não foi encontrado.")
        return False
    except NoCredentialsError:
        print("Credenciais incorretas ou indisponíveis.")
        return False
    return True

def main():
    for file_name in csv_files:
        file_path = os.path.join(folder_path, file_name)
        object_name = file_name
        if upload_to_s3(file_path, bucket_name, object_name):
            print(f"Arquivo {file_name} foi enviado com sucesso para o S3!")
        else:
            print(f"Falha ao enviar arquivo {file_name} para o S3.")

if __name__ == "__main__":
    main()