"""Create users table

Revision ID: a01e986b126c
Revises:
Create Date: 2019-01-15 20:00:11.596593

"""
from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision = 'a01e986b126c'
down_revision = None
branch_labels = None
depends_on = None


def upgrade():
    op.create_table(
        'users',
        sa.Column('id', sa.Integer, primary_key=True),
        sa.Column('email', sa.String, nullable=False, unique=True),
        sa.Column('password_hash', sa.String(60), nullable=False),
        sa.Column('token', sa.Binary(32)),
        sa.Column('token_expiration', sa.DateTime),
    )

def downgrade():
    op.drop_table('users')

